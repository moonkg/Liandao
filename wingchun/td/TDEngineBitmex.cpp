#include "TDEngineBitmex.h"
#include "longfist/ctp.h"
#include "longfist/LFUtils.h"
#include "TypeConvert.hpp"
#include <boost/algorithm/string.hpp>

#include <writer.h>
#include <stringbuffer.h>
#include <document.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <cpr/cpr.h>
#include <chrono>
#include "../../utils/crypto/openssl_util.h"

using cpr::Delete;
using cpr::Get;
using cpr::Url;
using cpr::Body;
using cpr::Header;
using cpr::Parameters;
using cpr::Payload;
using cpr::Post;
using cpr::Timeout;

using rapidjson::StringRef;
using rapidjson::Writer;
using rapidjson::StringBuffer;
using rapidjson::Document;
using rapidjson::SizeType;
using rapidjson::Value;
using std::string;
using std::to_string;
using std::stod;
using std::stoi;
using utils::crypto::hmac_sha256;
using utils::crypto::hmac_sha256_byte;
using utils::crypto::base64_encode;


USING_WC_NAMESPACE

TDEngineBitmex::TDEngineBitmex(): ITDEngine(SOURCE_BITMEX)
{
    logger = yijinjing::KfLog::getLogger("TradeEngine.Bitmex");
    KF_LOG_INFO(logger, "[TDEngineBitmex]");

    mutex_order_and_trade = new std::mutex();
}

TDEngineBitmex::~TDEngineBitmex()
{
    if(mutex_order_and_trade != nullptr) delete mutex_order_and_trade;
}

void TDEngineBitmex::init()
{
    ITDEngine::init();
    JournalPair tdRawPair = getTdRawJournalPair(source_id);
    raw_writer = yijinjing::JournalWriter::create(tdRawPair.first, tdRawPair.second, "RAW_" + name());
    KF_LOG_INFO(logger, "[init]");
}

void TDEngineBitmex::pre_load(const json& j_config)
{
    KF_LOG_INFO(logger, "[pre_load]");
}

void TDEngineBitmex::resize_accounts(int account_num)
{
    account_units.resize(account_num);
    KF_LOG_INFO(logger, "[resize_accounts]");
}

TradeAccount TDEngineBitmex::load_account(int idx, const json& j_config)
{
    KF_LOG_INFO(logger, "[load_account]");
    // internal load
    string api_key = j_config["APIKey"].get<string>();
    string secret_key = j_config["SecretKey"].get<string>();

    string baseUrl = j_config["baseUrl"].get<string>();
    rest_get_interval_ms = j_config["rest_get_interval_ms"].get<int>();


    AccountUnitBitmex& unit = account_units[idx];
    unit.api_key = api_key;
    unit.secret_key = secret_key;
    unit.baseUrl = baseUrl;

    KF_LOG_INFO(logger, "[load_account] (api_key)" << api_key << " (baseUrl)" << unit.baseUrl);

    unit.coinPairWhiteList.ReadWhiteLists(j_config, "whiteLists");
    unit.coinPairWhiteList.Debug_print();

    unit.positionWhiteList.ReadWhiteLists(j_config, "positionWhiteLists");
    unit.positionWhiteList.Debug_print();

    //display usage:
    if(unit.coinPairWhiteList.Size() == 0) {
        KF_LOG_ERROR(logger, "TDEngineBitmex::load_account: please add whiteLists in kungfu.json like this :");
        KF_LOG_ERROR(logger, "\"whiteLists\":{");
        KF_LOG_ERROR(logger, "    \"strategy_coinpair(base_quote)\": \"exchange_coinpair\",");
        KF_LOG_ERROR(logger, "    \"btc_usdt\": \"BTC_USDT\",");
        KF_LOG_ERROR(logger, "     \"etc_eth\": \"ETC_ETH\"");
        KF_LOG_ERROR(logger, "},");
    }

    //cancel all openning orders on TD startup
    Document d;
    cancel_all_orders(unit, d);

    // set up
    TradeAccount account = {};
    //partly copy this fields
    strncpy(account.UserID, api_key.c_str(), 16);
    strncpy(account.Password, secret_key.c_str(), 21);
    return account;
}


void TDEngineBitmex::connect(long timeout_nsec)
{
    KF_LOG_INFO(logger, "[connect]");
    for (int idx = 0; idx < account_units.size(); idx ++)
    {
        AccountUnitBitmex& unit = account_units[idx];
        KF_LOG_INFO(logger, "[connect] (api_key)" << unit.api_key);
        if (!unit.logged_in)
        {
            //exchange infos
            Document doc;
            //TODO
            get_products(unit, doc);
            KF_LOG_INFO(logger, "[connect] get_products");
            printResponse(doc);

            if(loadExchangeOrderFilters(unit, doc))
            {
                unit.logged_in = true;
            } else {
                KF_LOG_ERROR(logger, "[connect] logged_in = false for loadExchangeOrderFilters return false");
            }
            debug_print(unit.sendOrderFilters);
            unit.logged_in = true;
        }
    }
}

//TODO
bool TDEngineBitmex::loadExchangeOrderFilters(AccountUnitBitmex& unit, Document &doc)
{
    KF_LOG_INFO(logger, "[loadExchangeOrderFilters]");
    //changelog 2018-07-20. use hardcode mode
    /*
    BTC_USDT	0.0001		4
    ETH_USDT	0.0001		4
    LTC_USDT	0.0001		4
    BCH_USDT	0.0001		4
    ETC_USDT	0.0001		4
    ETC_ETH	0.00000001		8
    LTC_BTC	0.00000001		8
    BCH_BTC	0.00000001		8
    ETH_BTC	0.00000001		8
    ETC_BTC	0.00000001		8
     * */
    SendOrderFilter afilter;

    strncpy(afilter.InstrumentID, "BTC_USDT", 31);
    afilter.ticksize = 4;
    unit.sendOrderFilters.insert(std::make_pair("BTC_USDT", afilter));

    strncpy(afilter.InstrumentID, "ETH_USDT", 31);
    afilter.ticksize = 4;
    unit.sendOrderFilters.insert(std::make_pair("ETH_USDT", afilter));

    strncpy(afilter.InstrumentID, "LTC_USDT", 31);
    afilter.ticksize = 4;
    unit.sendOrderFilters.insert(std::make_pair("LTC_USDT", afilter));

    strncpy(afilter.InstrumentID, "BCH_USDT", 31);
    afilter.ticksize = 4;
    unit.sendOrderFilters.insert(std::make_pair("BCH_USDT", afilter));

    strncpy(afilter.InstrumentID, "ETC_USDT", 31);
    afilter.ticksize = 4;
    unit.sendOrderFilters.insert(std::make_pair("ETC_USDT", afilter));

    strncpy(afilter.InstrumentID, "ETC_ETH", 31);
    afilter.ticksize = 8;
    unit.sendOrderFilters.insert(std::make_pair("ETC_ETH", afilter));

    strncpy(afilter.InstrumentID, "LTC_BTC", 31);
    afilter.ticksize = 8;
    unit.sendOrderFilters.insert(std::make_pair("LTC_BTC", afilter));

    strncpy(afilter.InstrumentID, "BCH_BTC", 31);
    afilter.ticksize = 8;
    unit.sendOrderFilters.insert(std::make_pair("BCH_BTC", afilter));

    strncpy(afilter.InstrumentID, "ETH_BTC", 31);
    afilter.ticksize = 8;
    unit.sendOrderFilters.insert(std::make_pair("ETH_BTC", afilter));

    strncpy(afilter.InstrumentID, "ETC_BTC", 31);
    afilter.ticksize = 8;
    unit.sendOrderFilters.insert(std::make_pair("ETC_BTC", afilter));

    //parse bitmex json
    /*
     [{"baseCurrency":"LTC","baseMaxSize":"100000.00","baseMinSize":"0.001","code":"LTC_BTC","quoteCurrency":"BTC","quoteIncrement":"8"},
     {"baseCurrency":"BCH","baseMaxSize":"100000.00","baseMinSize":"0.001","code":"BCH_BTC","quoteCurrency":"BTC","quoteIncrement":"8"},
     {"baseCurrency":"ETH","baseMaxSize":"100000.00","baseMinSize":"0.001","code":"ETH_BTC","quoteCurrency":"BTC","quoteIncrement":"8"},
     {"baseCurrency":"ETC","baseMaxSize":"100000.00","baseMinSize":"0.01","code":"ETC_BTC","quoteCurrency":"BTC","quoteIncrement":"8"},
     ...
     ]
     * */
//    if(doc.HasParseError() || doc.IsObject())
//    {
//        return false;
//    }
//    if(doc.IsArray())
//    {
//        int symbolsCount = doc.Size();
//        for (int i = 0; i < symbolsCount; i++) {
//            const rapidjson::Value& sym = doc.GetArray()[i];
//            std::string symbol = sym["code"].GetString();
//            std::string tickSizeStr =  sym["baseMinSize"].GetString();
//            KF_LOG_INFO(logger, "[loadExchangeOrderFilters] sendOrderFilters (symbol)" << symbol <<
//                                                                                       " (tickSizeStr)" << tickSizeStr);
//            //0.0000100; 0.001;  1; 10
//            SendOrderFilter afilter;
//            strncpy(afilter.InstrumentID, symbol.c_str(), 31);
//            afilter.ticksize = Round(tickSizeStr);
//            unit.sendOrderFilters.insert(std::make_pair(symbol, afilter));
//            KF_LOG_INFO(logger, "[loadExchangeOrderFilters] sendOrderFilters (symbol)" << symbol <<
//                                                                                       " (tickSizeStr)" << tickSizeStr
//                                                                                       <<" (tickSize)" << afilter.ticksize);
//        }
//    }

    return true;
}

void TDEngineBitmex::debug_print(std::map<std::string, SendOrderFilter> &sendOrderFilters)
{
    std::map<std::string, SendOrderFilter>::iterator map_itr = sendOrderFilters.begin();
    while(map_itr != sendOrderFilters.end())
    {
        KF_LOG_INFO(logger, "[debug_print] sendOrderFilters (symbol)" << map_itr->first <<
                                                                      " (tickSize)" << map_itr->second.ticksize);
        map_itr++;
    }
}

SendOrderFilter TDEngineBitmex::getSendOrderFilter(AccountUnitBitmex& unit, const char *symbol)
{
    std::map<std::string, SendOrderFilter>::iterator map_itr = unit.sendOrderFilters.begin();
    while(map_itr != unit.sendOrderFilters.end())
    {
        if(strcmp(map_itr->first.c_str(), symbol) == 0)
        {
            return map_itr->second;
        }
        map_itr++;
    }
    SendOrderFilter defaultFilter;
    defaultFilter.ticksize = 8;
    strcpy(defaultFilter.InstrumentID, "notfound");
    return defaultFilter;
}

void TDEngineBitmex::login(long timeout_nsec)
{
    KF_LOG_INFO(logger, "[login]");
    connect(timeout_nsec);
}

void TDEngineBitmex::logout()
{
    KF_LOG_INFO(logger, "[logout]");
}

void TDEngineBitmex::release_api()
{
    KF_LOG_INFO(logger, "[release_api]");
}

bool TDEngineBitmex::is_logged_in() const
{
    KF_LOG_INFO(logger, "[is_logged_in]");
    for (auto& unit: account_units)
    {
        if (!unit.logged_in)
            return false;
    }
    return true;
}

bool TDEngineBitmex::is_connected() const
{
    KF_LOG_INFO(logger, "[is_connected]");
    return is_logged_in();
}



std::string TDEngineBitmex::GetSide(const LfDirectionType& input) {
    if (LF_CHAR_Buy == input) {
        return "buy";
    } else if (LF_CHAR_Sell == input) {
        return "sell";
    } else {
        return "";
    }
}

LfDirectionType TDEngineBitmex::GetDirection(std::string input) {
    if ("buy" == input) {
        return LF_CHAR_Buy;
    } else if ("sell" == input) {
        return LF_CHAR_Sell;
    } else {
        return LF_CHAR_Buy;
    }
}

std::string TDEngineBitmex::GetType(const LfOrderPriceTypeType& input) {
    if (LF_CHAR_LimitPrice == input) {
        return "limit";
    } else if (LF_CHAR_AnyPrice == input) {
        return "market";
    } else {
        return "";
    }
}

LfOrderPriceTypeType TDEngineBitmex::GetPriceType(std::string input) {
    if ("limit" == input) {
        return LF_CHAR_LimitPrice;
    } else if ("market" == input) {
        return LF_CHAR_AnyPrice;
    } else {
        return '0';
    }
}
//订单状态，﻿open（未成交）、filled（已完成）、canceled（已撤销）、cancel（撤销中）、partially-filled（部分成交）
LfOrderStatusType TDEngineBitmex::GetOrderStatus(std::string input) {
    if ("open" == input) {
        return LF_CHAR_NotTouched;
    } else if ("partially-filled" == input) {
        return LF_CHAR_PartTradedQueueing;
    } else if ("filled" == input) {
        return LF_CHAR_AllTraded;
    } else if ("canceled" == input) {
        return LF_CHAR_Canceled;
    } else if ("cancel" == input) {
        return LF_CHAR_NotTouched;
    } else {
        return LF_CHAR_NotTouched;
    }
}

/**
 * req functions
 */
void TDEngineBitmex::req_investor_position(const LFQryPositionField* data, int account_index, int requestId)
{
    KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId);

    AccountUnitBitmex& unit = account_units[account_index];
    KF_LOG_INFO(logger, "[req_investor_position] (api_key)" << unit.api_key << " (InstrumentID) " << data->InstrumentID);

    int errorId = 0;
    std::string errorMsg = "";
    Document d;
    get_account(unit, d);

    if(d.IsObject() && d.HasMember("code"))
    {
        errorId = d["code"].GetInt();
        if(d.HasMember("message") && d["message"].IsString())
        {
            errorMsg = d["message"].GetString();
        }
        KF_LOG_ERROR(logger, "[req_investor_position] failed!" << " (rid)" << requestId << " (errorId)" << errorId << " (errorMsg) " << errorMsg);
    }
    send_writer->write_frame(data, sizeof(LFQryPositionField), source_id, MSG_TYPE_LF_QRY_POS_COINMEX, 1, requestId);

    LFRspPositionField pos;
    memset(&pos, 0, sizeof(LFRspPositionField));
    strncpy(pos.BrokerID, data->BrokerID, 11);
    strncpy(pos.InvestorID, data->InvestorID, 19);
    strncpy(pos.InstrumentID, data->InstrumentID, 31);
    pos.PosiDirection = LF_CHAR_Long;
    pos.HedgeFlag = LF_CHAR_Speculation;
    pos.Position = 0;
    pos.YdPosition = 0;
    pos.PositionCost = 0;


/*
 # Response
    [{"available":"0.099","balance":"0.099","currencyCode":"BTC","hold":"0","id":83906},{"available":"188","balance":"188","currencyCode":"MVP","hold":"0","id":83906}]
 * */
    std::vector<LFRspPositionField> tmp_vector;
    if(d.IsArray())
    {
        size_t len = d.Size();
        KF_LOG_INFO(logger, "[req_investor_position] (asset.length)" << len);
        for(int i = 0; i < len; i++)
        {
            std::string symbol = d.GetArray()[i]["currencyCode"].GetString();
            std::string ticker = unit.positionWhiteList.GetKeyByValue(symbol);
            if(ticker.length() > 0) {
                strncpy(pos.InstrumentID, ticker.c_str(), 31);
                pos.Position = std::round(std::stod(d.GetArray()[i]["available"].GetString()) * scale_offset);
                tmp_vector.push_back(pos);
                KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol
                                                                          << " available:" << d.GetArray()[i]["available"].GetString()
                                                                          << " balance: " << d.GetArray()[i]["balance"].GetString()
                                                                          << " hold: " << d.GetArray()[i]["hold"].GetString());
                KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol << " (position) " << pos.Position);
            }
        }
    }

    bool findSymbolInResult = false;
    //send the filtered position
    int position_count = tmp_vector.size();
    for (int i = 0; i < position_count; i++)
    {
        on_rsp_position(&tmp_vector[i], i == (position_count - 1), requestId, errorId, errorMsg.c_str());
        findSymbolInResult = true;
    }

    if(!findSymbolInResult)
    {
        KF_LOG_INFO(logger, "[req_investor_position] (!findSymbolInResult) (requestId)" << requestId);
        on_rsp_position(&pos, 1, requestId, errorId, errorMsg.c_str());
    }
    if(errorId != 0)
    {
        raw_writer->write_error_frame(&pos, sizeof(LFRspPositionField), source_id, MSG_TYPE_LF_RSP_POS_COINMEX, 1, requestId, errorId, errorMsg.c_str());
    }
}

void TDEngineBitmex::req_qry_account(const LFQryAccountField *data, int account_index, int requestId)
{
    KF_LOG_INFO(logger, "[req_qry_account]");
}

int64_t TDEngineBitmex::fixPriceTickSize(int keepPrecision, int64_t price, bool isBuy) {
    if(keepPrecision == 8) return price;

    int removePrecisions = (8 - keepPrecision);
    double cutter = pow(10, removePrecisions);

    KF_LOG_INFO(logger, "[fixPriceTickSize input]" << " 1(price)" << std::fixed  << std::setprecision(9) << price);
    double new_price = price/cutter;
    KF_LOG_INFO(logger, "[fixPriceTickSize input]" << " 2(price/cutter)" << std::fixed  << std::setprecision(9) << new_price);
    if(isBuy){
        new_price += 0.9;
        new_price = std::floor(new_price);
        KF_LOG_INFO(logger, "[fixPriceTickSize input]" << " 3(price is buy)" << std::fixed  << std::setprecision(9) << new_price);
    } else {
        new_price = std::floor(new_price);
        KF_LOG_INFO(logger, "[fixPriceTickSize input]" << " 3(price is sell)" << std::fixed  << std::setprecision(9) << new_price);
    }
    int64_t  ret_price = new_price * cutter;
    KF_LOG_INFO(logger, "[fixPriceTickSize input]" << " 4(new_price * cutter)" << std::fixed  << std::setprecision(9) << new_price);
    return ret_price;
}

int TDEngineBitmex::Round(std::string tickSizeStr) {
    size_t docAt = tickSizeStr.find( ".", 0 );
    size_t oneAt = tickSizeStr.find( "1", 0 );

    if(docAt == string::npos) {
        //not ".", it must be "1" or "10"..."100"
        return -1 * (tickSizeStr.length() -  1);
    }
    //there must exist 1 in the string.
    return oneAt - docAt;
}


void TDEngineBitmex::req_order_insert(const LFInputOrderField* data, int account_index, int requestId, long rcv_time)
{
    AccountUnitBitmex& unit = account_units[account_index];
    KF_LOG_DEBUG(logger, "[req_order_insert]" << " (rid)" << requestId
                                              << " (APIKey)" << unit.api_key
                                              << " (Tid)" << data->InstrumentID
                                              << " (Volume)" << data->Volume
                                              << " (LimitPrice)" << data->LimitPrice
                                              << " (OrderRef)" << data->OrderRef);
    send_writer->write_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER_COINMEX, 1/*ISLAST*/, requestId);

    int errorId = 0;
    std::string errorMsg = "";

    std::string ticker = unit.coinPairWhiteList.GetValueByKey(std::string(data->InstrumentID));
    if(ticker.length() == 0) {
        errorId = 200;
        errorMsg = std::string(data->InstrumentID) + " not in WhiteList, ignore it";
        KF_LOG_ERROR(logger, "[req_order_insert]: not in WhiteList, ignore it  (rid)" << requestId <<
                                                                                      " (errorId)" << errorId << " (errorMsg) " << errorMsg);
        on_rsp_order_insert(data, requestId, errorId, errorMsg.c_str());
        raw_writer->write_error_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER_COINMEX, 1, requestId, errorId, errorMsg.c_str());
        return;
    }
    KF_LOG_DEBUG(logger, "[req_order_insert] (exchange_ticker)" << ticker);

    double funds = 0;
    Document d;

    SendOrderFilter filter = getSendOrderFilter(unit, ticker.c_str());

    int64_t fixedPrice = fixPriceTickSize(filter.ticksize, data->LimitPrice, LF_CHAR_Buy == data->Direction);

    KF_LOG_DEBUG(logger, "[req_order_insert] SendOrderFilter  (Tid)" << ticker <<
                                                                     " (LimitPrice)" << data->LimitPrice <<
                                                                     " (ticksize)" << filter.ticksize <<
                                                                     " (fixedPrice)" << fixedPrice);

    send_order(unit, ticker.c_str(), GetSide(data->Direction).c_str(),
            GetType(data->OrderPriceType).c_str(), data->Volume*1.0/scale_offset, fixedPrice*1.0/scale_offset, data->OrderRef, d);

    //not expected response
    if(d.HasParseError() || !d.IsObject())
    {
        errorId = 100;
        errorMsg = "send_order http response has parse error or is not json. please check the log";
        KF_LOG_ERROR(logger, "[req_order_insert] send_order error!  (rid)" << requestId << " (errorId)" <<
                                                                           errorId << " (errorMsg) " << errorMsg);
    } else  if(d.HasMember("orderId") && d.HasMember("result"))
    {
        if(d["result"].GetBool())
        {
            /*
             * # Response OK
                {
                    "result": true,
                    "order_id": 123456
                }
             * */
            //if send successful and the exchange has received ok, then add to  pending query order list
            std::string remoteOrderId = std::to_string(d["orderId"].GetInt64());
            localOrderRefRemoteOrderId.insert(std::make_pair(std::string(data->OrderRef), remoteOrderId));
            KF_LOG_INFO(logger, "[req_order_insert] after send  (rid)" << requestId << " (OrderRef) " <<
                                                                       data->OrderRef << " (remoteOrderId) " << remoteOrderId);

            char noneStatus = '\0';//none
            addNewQueryOrdersAndTrades(unit, data->InstrumentID, data->OrderRef, noneStatus, 0);
            //success, only record raw data
            raw_writer->write_error_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER_COINMEX, 1, requestId, errorId, errorMsg.c_str());
            return;
        } else {
            /*
             * # Response error
                {
                    "result": false,
                    "order_id": 123456
                }
             * */
            //send successful BUT the exchange has received fail
            errorId = 200;
            errorMsg = "http.code is 200, but result is false";
            KF_LOG_ERROR(logger, "[req_order_insert] send_order error!  (rid)" << requestId << " (errorId)" <<
                                                                               errorId << " (errorMsg) " << errorMsg);
        }
    } else if (d.HasMember("code") && d["code"].IsNumber()) {
        //send error, example: http timeout.
        errorId = d["code"].GetInt();
        if(d.HasMember("message") && d["message"].IsString())
        {
            errorMsg = d["message"].GetString();
        }
        KF_LOG_ERROR(logger, "[req_order_insert] failed!" << " (rid)" << requestId << " (errorId)" <<
                                                          errorId << " (errorMsg) " << errorMsg);
    }

    if(errorId != 0)
    {
        on_rsp_order_insert(data, requestId, errorId, errorMsg.c_str());
    }
    raw_writer->write_error_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER_COINMEX, 1, requestId, errorId, errorMsg.c_str());
}


void TDEngineBitmex::req_order_action(const LFOrderActionField* data, int account_index, int requestId, long rcv_time)
{
    AccountUnitBitmex& unit = account_units[account_index];
    KF_LOG_DEBUG(logger, "[req_order_action]" << " (rid)" << requestId
                                              << " (APIKey)" << unit.api_key
                                              << " (Iid)" << data->InvestorID
                                              << " (OrderRef)" << data->OrderRef
                                              << " (KfOrderID)" << data->KfOrderID);

    send_writer->write_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_COINMEX, 1, requestId);

    int errorId = 0;
    std::string errorMsg = "";

    std::string ticker = unit.coinPairWhiteList.GetValueByKey(std::string(data->InstrumentID));
    if(ticker.length() == 0) {
        errorId = 200;
        errorMsg = std::string(data->InstrumentID) + " not in WhiteList, ignore it";
        KF_LOG_ERROR(logger, "[req_order_action]: not in WhiteList , ignore it: (rid)" << requestId << " (errorId)" <<
                                                                                       errorId << " (errorMsg) " << errorMsg);
        on_rsp_order_action(data, requestId, errorId, errorMsg.c_str());
        raw_writer->write_error_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_COINMEX, 1, requestId, errorId, errorMsg.c_str());
        return;
    }
    KF_LOG_DEBUG(logger, "[req_order_action] (exchange_ticker)" << ticker);

    std::map<std::string, std::string>::iterator itr = localOrderRefRemoteOrderId.find(data->OrderRef);
    std::string remoteOrderId;
    if(itr == localOrderRefRemoteOrderId.end()) {
        errorId = 1;
        std::stringstream ss;
        ss << "[req_order_action] not found in localOrderRefRemoteOrderId map (orderRef) " << data->OrderRef;
        errorMsg = ss.str();
        KF_LOG_ERROR(logger, "[req_order_action] not found in localOrderRefRemoteOrderId map. "
                             << " (rid)" << requestId << " (orderRef)" << data->OrderRef << " (errorId)" << errorId << " (errorMsg) " << errorMsg);
        on_rsp_order_action(data, requestId, errorId, errorMsg.c_str());
        raw_writer->write_error_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_COINMEX, 1, requestId, errorId, errorMsg.c_str());
        return;
    } else {
        remoteOrderId = itr->second;
        KF_LOG_DEBUG(logger, "[req_order_action] found in localOrderRefRemoteOrderId map (orderRef) "
                             << data->OrderRef << " (remoteOrderId) " << remoteOrderId);
    }

    Document d;
    cancel_order(unit, stod(remoteOrderId), d);

    //cancel order response "" as resultText, it cause json.HasParseError() == true, and json.IsObject() == false.
    //it is not an error, so dont check it.
    //not expected response
    if(!d.HasParseError() && d.HasMember("code") && d["code"].IsNumber()) {
        errorId = d["code"].GetInt();
        if(d.HasMember("message") && d["message"].IsString())
        {
            errorMsg = d["message"].GetString();
        }
        KF_LOG_ERROR(logger, "[req_order_action] cancel_order failed!" << " (rid)" << requestId
                                                                       << " (errorId)" << errorId << " (errorMsg) " << errorMsg);
    }

    if(errorId != 0)
    {
        on_rsp_order_action(data, requestId, errorId, errorMsg.c_str());
    }
    raw_writer->write_error_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_COINMEX, 1, requestId, errorId, errorMsg.c_str());
}

void TDEngineBitmex::GetAndHandleOrderTradeResponse()
{
    //every account
    for (int idx = 0; idx < account_units.size(); idx ++)
    {
        AccountUnitBitmex& unit = account_units[idx];
        if (!unit.logged_in)
        {
            continue;
        }
        moveNewtoPending(unit);
        retrieveOrderStatus(unit);
    }//end every account
}


void TDEngineBitmex::moveNewtoPending(AccountUnitBitmex& unit)
{
    std::lock_guard<std::mutex> guard_mutex(*mutex_order_and_trade);

    std::vector<PendingBitmexOrderStatus>::iterator newOrderStatusIterator;
    for(newOrderStatusIterator = unit.newOrderStatus.begin(); newOrderStatusIterator != unit.newOrderStatus.end();)
    {
        unit.pendingOrderStatus.push_back(*newOrderStatusIterator);
        newOrderStatusIterator = unit.newOrderStatus.erase(newOrderStatusIterator);
    }
}

void TDEngineBitmex::retrieveOrderStatus(AccountUnitBitmex& unit)
{
    KF_LOG_INFO(logger, "[retrieveOrderStatus] ");
    std::vector<PendingBitmexOrderStatus>::iterator orderStatusIterator;
//    int indexNum = 0;
//    for(orderStatusIterator = unit.pendingOrderStatus.begin(); orderStatusIterator != unit.pendingOrderStatus.end(); orderStatusIterator++)
//    {
//        indexNum++;
//        KF_LOG_INFO(logger, "[retrieveOrderStatus] get_order [" << indexNum <<"]    (account.api_key)"<< unit.api_key
//                                                                << "  (account.pendingOrderStatus.InstrumentID) "<< orderStatusIterator->InstrumentID
//                                                                <<"  (account.pendingOrderStatus.OrderRef) " << orderStatusIterator->OrderRef
//                                                                <<"  (account.pendingOrderStatus.OrderStatus) " << orderStatusIterator->OrderStatus
//        );
//    }

    for(orderStatusIterator = unit.pendingOrderStatus.begin(); orderStatusIterator != unit.pendingOrderStatus.end();)
    {
        KF_LOG_INFO(logger, "[retrieveOrderStatus] get_order " << "( account.api_key) "<< unit.api_key
                                                               << "  (account.pendingOrderStatus.InstrumentID) "<< orderStatusIterator->InstrumentID
                                                               <<"  (account.pendingOrderStatus.OrderRef) " << orderStatusIterator->OrderRef
                                                               <<"  (account.pendingOrderStatus.OrderStatus) " << orderStatusIterator->OrderStatus
        );


        std::map<std::string, std::string>::iterator itr = localOrderRefRemoteOrderId.find(orderStatusIterator->OrderRef);
        std::string remoteOrderId;
        if(itr == localOrderRefRemoteOrderId.end()) {
            KF_LOG_ERROR(logger, "[retrieveOrderStatus] not found in localOrderRefRemoteOrderId map (orderRef) " << orderStatusIterator->OrderRef);
            continue;
        } else {
            remoteOrderId = itr->second;
            KF_LOG_INFO(logger, "[retrieveOrderStatus] found in localOrderRefRemoteOrderId map (orderRef) " << orderStatusIterator->OrderRef << " (remoteOrderId) " << remoteOrderId);
        }

        std::string ticker = unit.coinPairWhiteList.GetValueByKey(std::string(orderStatusIterator->InstrumentID));
        if(ticker.length() == 0) {
            KF_LOG_INFO(logger, "[retrieveOrderStatus]: not in WhiteList , ignore it:" << orderStatusIterator->InstrumentID);
            continue;
        }
        KF_LOG_DEBUG(logger, "[retrieveOrderStatus] (exchange_ticker)" << ticker);

        Document d;
        query_order(unit, ticker, stod(remoteOrderId), d);

        /*
 # Response

{
	"averagePrice": "0",
	"code": "MVP_BTC",
	"createdDate": 1530417365000,
	"filledVolume": "0",
	"funds": "0",
	"orderId": 20283535,
	"orderType": "limit",
	"price": "0.00000001",
	"side": "buy",
	"status": "open",
	"volume": "1"
}

返回值说明
返回字段 	字段说明
averagePrice 	订单已成交部分均价，如果未成交则为0
code 	币对如btc-usdt
createDate 	创建订单的时间戳
filledVolume 	订单已成交数量
funds 	订单已成交金额
orderId 	订单代码
price 	订单委托价
side 	订单交易方向
status 	订单状态
volume 	订单委托数量
        */
        //parse order status
        //订单状态，﻿open（未成交）、filled（已完成）、canceled（已撤销）、cancel（撤销中）、partially-filled（部分成交）
        if(d.HasParseError()) {
            //HasParseError, skip
            KF_LOG_ERROR(logger, "[retrieveOrderStatus] get_order response HasParseError " << " (symbol)" << orderStatusIterator->InstrumentID
                                                                   << " (orderRef)" << orderStatusIterator->OrderRef
                                                                   << " (remoteOrderId) " << remoteOrderId);
            continue;
        }
        if(d.HasMember("status"))
        {
            /*
             {
                "averagePrice": "0.00000148",
                "code": "MVP_BTC",
                "createdDate": 1530439964000,
                "filledVolume": "1",
                "funds": "0",
                "orderId": 20644648,
                "orderType": "limit",
                "price": "0.00001111",
                "side": "buy",
                "status": "filled",
                "volume": "1"
            }
             * */
            //parse success
            LFRtnOrderField rtn_order;
            memset(&rtn_order, 0, sizeof(LFRtnOrderField));
            rtn_order.OrderStatus = GetOrderStatus(d["status"].GetString());
            rtn_order.VolumeTraded = std::round(std::stod(d["filledVolume"].GetString()) * scale_offset);

            //if status changed or LF_CHAR_PartTradedQueueing but traded valume changes, emit onRtnOrder
            if(orderStatusIterator->OrderStatus != rtn_order.OrderStatus ||
               (LF_CHAR_PartTradedQueueing == rtn_order.OrderStatus
                && rtn_order.VolumeTraded != orderStatusIterator->VolumeTraded))
            {
                //first send onRtnOrder about the status change or VolumeTraded change
                strcpy(rtn_order.ExchangeID, "bitmex");
                strncpy(rtn_order.UserID, unit.api_key.c_str(), 16);
                strncpy(rtn_order.InstrumentID, orderStatusIterator->InstrumentID, 31);
                rtn_order.Direction = GetDirection(d["side"].GetString());
                //No this setting on bitmex
                rtn_order.TimeCondition = LF_CHAR_GFD;
                rtn_order.OrderPriceType = GetPriceType(d["orderType"].GetString());
                strncpy(rtn_order.OrderRef, orderStatusIterator->OrderRef, 13);
                rtn_order.VolumeTotalOriginal = std::round(std::stod(d["volume"].GetString()) * scale_offset);
                rtn_order.LimitPrice = std::round(std::stod(d["price"].GetString()) * scale_offset);
                rtn_order.VolumeTotal = rtn_order.VolumeTotalOriginal - rtn_order.VolumeTraded;

                on_rtn_order(&rtn_order);
                raw_writer->write_frame(&rtn_order, sizeof(LFRtnOrderField),
                                        source_id, MSG_TYPE_LF_RTN_ORDER_COINMEX,
                                        1, (rtn_order.RequestID > 0) ? rtn_order.RequestID: -1);

                uint64_t newAveragePrice = std::round(std::stod(d["averagePrice"].GetString()) * scale_offset);
                //second, if the status is PartTraded/AllTraded, send OnRtnTrade
                if(rtn_order.OrderStatus == LF_CHAR_AllTraded ||
                    (LF_CHAR_PartTradedQueueing == rtn_order.OrderStatus
                    && rtn_order.VolumeTraded != orderStatusIterator->VolumeTraded))
                {
                    LFRtnTradeField rtn_trade;
                    memset(&rtn_trade, 0, sizeof(LFRtnTradeField));
                    strcpy(rtn_trade.ExchangeID, "bitmex");
                    strncpy(rtn_trade.UserID, unit.api_key.c_str(), 16);
                    strncpy(rtn_trade.InstrumentID, orderStatusIterator->InstrumentID, 31);
                    strncpy(rtn_trade.OrderRef, orderStatusIterator->OrderRef, 13);
                    rtn_trade.Direction = rtn_order.Direction;
                    uint64_t oldAmount = orderStatusIterator->VolumeTraded * orderStatusIterator->averagePrice;
                    uint64_t newAmount = rtn_order.VolumeTraded * newAveragePrice;

                    //calculate the volumn and price (it is average too)
                    rtn_trade.Volume = rtn_order.VolumeTraded - orderStatusIterator->VolumeTraded;
                    rtn_trade.Price = (newAmount - oldAmount)/(rtn_order.VolumeTraded - orderStatusIterator->VolumeTraded);

                    on_rtn_trade(&rtn_trade);
                    raw_writer->write_frame(&rtn_trade, sizeof(LFRtnTradeField),
                                            source_id, MSG_TYPE_LF_RTN_TRADE_COINMEX, 1, -1);
                }
                //third, update last status for next query_order
                orderStatusIterator->OrderStatus = rtn_order.OrderStatus;
                orderStatusIterator->VolumeTraded = rtn_order.VolumeTraded;
                orderStatusIterator->averagePrice = newAveragePrice;
            }
        } else {
            int errorId = 0;
            std::string errorMsg = "";
            //no status, it must be a Error response. see details in getResponse(...)
            if(d.HasMember("code") && d["code"].IsInt()) {
                errorId = d["code"].GetInt();
            }
            if(d.HasMember("message") && d["message"].IsString())
            {
                errorMsg = d["message"].GetString();
            }
            KF_LOG_ERROR(logger, "[retrieveOrderStatus] get_order fail." << " (symbol)" << orderStatusIterator->InstrumentID
                                                                         << " (orderRef)" << orderStatusIterator->OrderRef
                                                                         << " (errorId)" << errorId
                                                                         << " (errorMsg)" << errorMsg);
        }

        //remove order when finish
        if(orderStatusIterator->OrderStatus == LF_CHAR_AllTraded  || orderStatusIterator->OrderStatus == LF_CHAR_Canceled
           || orderStatusIterator->OrderStatus == LF_CHAR_Error)
        {
            KF_LOG_INFO(logger, "[retrieveOrderStatus] remove a pendingOrderStatus.");
            orderStatusIterator = unit.pendingOrderStatus.erase(orderStatusIterator);
        } else {
            ++orderStatusIterator;
        }
        KF_LOG_INFO(logger, "[retrieveOrderStatus] move to next pendingOrderStatus.");
    }
}


void TDEngineBitmex::addNewQueryOrdersAndTrades(AccountUnitBitmex& unit, const char_31 InstrumentID,
                                                 const char_21 OrderRef, const LfOrderStatusType OrderStatus, const uint64_t VolumeTraded)
{
    //add new orderId for GetAndHandleOrderTradeResponse
    std::lock_guard<std::mutex> guard_mutex(*mutex_order_and_trade);

    PendingBitmexOrderStatus status;
    memset(&status, 0, sizeof(PendingBitmexOrderStatus));
    strncpy(status.InstrumentID, InstrumentID, 31);
    strncpy(status.OrderRef, OrderRef, 21);
    status.OrderStatus = OrderStatus;
    status.VolumeTraded = VolumeTraded;
    status.averagePrice = 0.0;
    unit.newOrderStatus.push_back(status);
    KF_LOG_INFO(logger, "[addNewQueryOrdersAndTrades] (InstrumentID) " << InstrumentID
                                                                       << " (OrderRef) " << OrderRef
                                                                       << "(VolumeTraded)" << VolumeTraded);
}


void TDEngineBitmex::set_reader_thread()
{
    ITDEngine::set_reader_thread();

    KF_LOG_INFO(logger, "[set_reader_thread] rest_thread start on TDEngineBitmex::loop");
    rest_thread = ThreadPtr(new std::thread(boost::bind(&TDEngineBitmex::loop, this)));

}

void TDEngineBitmex::loop()
{
    while(isRunning)
    {
        using namespace std::chrono;
        auto current_ms = duration_cast< milliseconds>(system_clock::now().time_since_epoch()).count();
        if(last_rest_get_ts != 0 && (current_ms - last_rest_get_ts) < rest_get_interval_ms)
        {
            continue;
        }

        last_rest_get_ts = current_ms;
        GetAndHandleOrderTradeResponse();
    }
}


std::vector<std::string> TDEngineBitmex::split(std::string str, std::string token)
{
    std::vector<std::string>result;
    while (str.size()) {
        size_t index = str.find(token);
        if (index != std::string::npos) {
            result.push_back(str.substr(0, index));
            str = str.substr(index + token.size());
            if (str.size() == 0)result.push_back(str);
        }
        else {
            result.push_back(str);
            str = "";
        }
    }
    return result;
}

void TDEngineBitmex::printResponse(const Document& d)
{
    if(d.IsObject() && d.HasMember("code")) {
        KF_LOG_INFO(logger, "[printResponse] error (code) " << d["code"].GetInt() << " (message) " << d["message"].GetString());
    } else {
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        d.Accept(writer);
        KF_LOG_INFO(logger, "[printResponse] ok (text) " << buffer.GetString());
    }
}

/*
 * https://github.com/bitmex/bitmex-official-api-docs/blob/master/README_ZH_CN.md
 *
 *成功
HTTP状态码200表示成功响应，并可能包含内容。如果响应含有内容，则将显示在相应的返回内容里面。
常见错误码
    400 Bad Request – Invalid request forma 请求格式无效
    401 Unauthorized – Invalid API Key 无效的API Key
    403 Forbidden – You do not have access to the requested resource 请求无权限
    404 Not Found 没有找到请求
    429 Too Many Requests 请求太频繁被系统限流
    500 Internal Server Error – We had a problem with our server 服务器内部阻碍
 * */
//当出错时，返回http error code和出错信息message
//当不出错时，返回结果信息
void TDEngineBitmex::getResponse(int http_status_code, std::string responseText, std::string errorMsg, Document& json)
{
    if(http_status_code == HTTP_RESPONSE_OK)
    {
        KF_LOG_INFO(logger, "[getResponse] (http_status_code == 200) (responseText)" << responseText << " (errorMsg) " << errorMsg);
        json.Parse(responseText.c_str());
        KF_LOG_INFO(logger, "[getResponse] (http_status_code == 200) (HasParseError)" << json.HasParseError());
    } else if(http_status_code == 0 && responseText.length() == 0)
    {
        json.SetObject();
        Document::AllocatorType& allocator = json.GetAllocator();
        int errorId = 1;
        json.AddMember("code", errorId, allocator);
        //KF_LOG_INFO(logger, "[getResponse] (errorMsg)" << errorMsg);
        rapidjson::Value val;
        val.SetString(errorMsg.c_str(), errorMsg.length(), allocator);
        json.AddMember("message", val, allocator);
    } else
    {
        Document d;
        d.Parse(responseText.c_str());
        //KF_LOG_INFO(logger, "[getResponse] (err) (responseText)" << responseText.c_str());

        json.SetObject();
        Document::AllocatorType& allocator = json.GetAllocator();
        json.AddMember("code", http_status_code, allocator);
        if(!d.HasParseError() && d.IsObject()) {
            if( d.HasMember("message")) {
                //KF_LOG_INFO(logger, "[getResponse] (err) (errorMsg)" << d["message"].GetString());
                std::string message = d["message"].GetString();
                rapidjson::Value val;
                val.SetString(message.c_str(), message.length(), allocator);
                json.AddMember("message", val, allocator);
            }
            if( d.HasMember("msg")) {
                //KF_LOG_INFO(logger, "[getResponse] (err) (errorMsg)" << d["msg"].GetString());
                std::string message = d["msg"].GetString();
                rapidjson::Value val;
                val.SetString(message.c_str(), message.length(), allocator);
                json.AddMember("message", val, allocator);
            }
        } else {
            rapidjson::Value val;
            val.SetString(errorMsg.c_str(), errorMsg.length(), allocator);
            json.AddMember("message", val, allocator);
        }
    }
}


void TDEngineBitmex::get_account(AccountUnitBitmex& unit, Document& json)
{
    KF_LOG_INFO(logger, "[get_account]");
    std::string Timestamp = getTimestampString();
    std::string Method = "GET";
    std::string requestPath = "/api/v1/spot/ccex/account/assets";
    std::string queryString= "";
    std::string body = "";
    string Message = Timestamp + Method + requestPath + queryString + body;

    unsigned char* signature = hmac_sha256_byte(unit.secret_key.c_str(), Message.c_str());
    string url = unit.baseUrl + requestPath;
    std::string sign = base64_encode(signature, 32);

    const auto response = Get(Url{url},
                              Header{{"ACCESS-KEY", unit.api_key},
                                     {"Content-Type", "application/json"},
                                     {"ACCESS-SIGN", sign},
                                     {"ACCESS-TIMESTAMP",  Timestamp}}, Timeout{10000} );

    KF_LOG_INFO(logger, "[get_account] (url) " << url << " (response) " << response.text.c_str());
    //[]
    return getResponse(response.status_code, response.text, response.error.message, json);
}

void TDEngineBitmex::get_products(AccountUnitBitmex& unit, Document& json)
{
 /*
[{
	"baseCurrency": "LTC",
	"baseMaxSize": "100000.00",
	"baseMinSize": "0.001",
	"code": "LTC_BTC",
	"quoteCurrency": "BTC",
	"quoteIncrement": "8"
}, {
	"baseCurrency": "BCH",
	"baseMaxSize": "100000.00",
	"baseMinSize": "0.001",
	"code": "BCH_BTC",
	"quoteCurrency": "BTC",
	"quoteIncrement": "8"
}, {
	"baseCurrency": "ETH",
	"baseMaxSize": "100000.00",
	"baseMinSize": "0.001",
	"code": "ETH_BTC",
	"quoteCurrency": "BTC",
	"quoteIncrement": "8"
}.......]
  * */
    KF_LOG_INFO(logger, "[get_products]");
    std::string Timestamp = getTimestampString();
    std::string Method = "GET";
    std::string requestPath = "/api/v1/instrument";
    std::string queryString= "";
    std::string body = "";
    string Message = Method + requestPath + Timestamp + queryString + body;

    unsigned char* signature = hmac_sha256_byte(unit.secret_key.c_str(), Message.c_str());
    string url = unit.baseUrl + requestPath;
    std::string sign = base64_encode(signature, 32);
    const auto response = Get(Url{url},
                              Header{{"ACCESS-KEY", unit.api_key},
                                     {"Content-Type", "application/json"},
                                     {"ACCESS-SIGN", sign},
                                     {"ACCESS-TIMESTAMP",  Timestamp}}, Timeout{10000} );

    KF_LOG_INFO(logger, "[get_products] (url) " << url << " (response) " << response.text.c_str());
    //
    return getResponse(response.status_code, response.text, response.error.message, json);
}

//https://www.bitmex.com/api/explorer/#!/Order/Order_new

void TDEngineBitmex::send_order(AccountUnitBitmex& unit, const char *code,
                                     const char *side, const char *type, double size, double price,  std::string orderRef,  Document& json)
{
    KF_LOG_INFO(logger, "[send_order]");

    std::string priceStr;
    std::stringstream convertPriceStream;
    convertPriceStream <<std::fixed << std::setprecision(8) << price;
    convertPriceStream >> priceStr;

    std::string sizeStr;
    std::stringstream convertSizeStream;
    convertSizeStream <<std::fixed << std::setprecision(8) << size;
    convertSizeStream >> sizeStr;

    KF_LOG_INFO(logger, "[send_order] (code) " << code << " (side) "<< side << " (type) " <<
                                               type << " (size) "<< sizeStr << " (price) "<< priceStr);

    Document document;
    document.SetObject();
    Document::AllocatorType& allocator = document.GetAllocator();
    //used inner this method only.so  can use reference
    /*Instrument symbol. e.g. 'XBTUSD'.
    * */
    document.AddMember("symbol", StringRef(code), allocator);
    /*Order side. Valid options: Buy, Sell. Defaults to 'Buy' unless orderQty or simpleOrderQty is negative.
    * */
    document.AddMember("side", StringRef(side), allocator);
    /*Order type. Valid options: Market, Limit, Stop, StopLimit, MarketIfTouched, LimitIfTouched, MarketWithLeftOverAsLimit, Pegged. Defaults to 'Limit' when price is specified. Defaults to 'Stop' when stopPx is specified. Defaults to 'StopLimit' when price and stopPx are specified.
     * */
    document.AddMember("ordType", StringRef(type), allocator);
    /*simpleOrderQty:   Order quantity in units of the underlying instrument (i.e. Bitcoin).
     *
     * orderQty: Order quantity in units of the instrument (i.e. contracts).
     * */
    document.AddMember("orderQty", StringRef(sizeStr.c_str()), allocator);
    /*
     * Optional limit price for 'Limit', 'StopLimit', and 'LimitIfTouched' orders.
     * */
    document.AddMember("price", StringRef(priceStr.c_str()), allocator);
    /*
     * clOrdID : Optional Client Order ID. This clOrdID will come back on the order and any related executions.
     * */
    document.AddMember("clOrdID", StringRef(orderRef.c_str()), allocator);

    StringBuffer jsonStr;
    Writer<StringBuffer> writer(jsonStr);
    document.Accept(writer);

    std::string Timestamp = getTimestampString();
    std::string Method = "POST";
    std::string requestPath = "/api/v1/order";
    std::string queryString= "";
    std::string body = jsonStr.GetString();

    std::time_t baseNow = std::time(nullptr);
    struct tm* tm = std::localtime(&baseNow);
    tm->tm_sec += 30;
    std::time_t next = std::mktime(tm);

    string Message = Timestamp + Method + requestPath + queryString + body;
    std::string signature = hmac_sha256(unit.secret_key.c_str(), Message.c_str());
    string url = unit.baseUrl + requestPath + queryString;

    /*
     * Header{{"api-key", unit.api_key},
                                      {"Accept", "application/json"},
                                      {"Content-Type", "application/x-www-form-urlencoded"},
                                      {"Content-Length", to_string(body.size())},
                                      {"api-signature", signature},
                                      {"api-expires", std::to_string(next) }},
     * */
    const auto response = Post(Url{url},
                               Header{{"api-key", unit.api_key},
                                      {"Accept", "application/json"},
                                      {"Content-Type", "application/x-www-form-urlencoded"},
                                      {"Content-Length", to_string(body.size())},
                                      {"api-signature", signature},
                                      {"api-expires", std::to_string(next) }},
                               Body{body}, Timeout{30000});

    //an error:
    //(response.status_code) 0 (response.error.message) Failed to connect to www.bitmore.top port 443: Connection refused (response.text)
    KF_LOG_INFO(logger, "[send_order] (url) " << url << " (body) "<< body << " (response.status_code) " << response.status_code <<
                                              " (response.error.message) " << response.error.message <<
                                              " (response.text) " << response.text.c_str());
    getResponse(response.status_code, response.text, response.error.message, json);
}


void TDEngineBitmex::cancel_all_orders(AccountUnitBitmex& unit, Document& json)
{
    KF_LOG_INFO(logger, "[cancel_all_orders]");

    std::time_t baseNow = std::time(nullptr);
    struct tm* tm = std::localtime(&baseNow);
    tm->tm_sec += 30;
    std::time_t next = std::mktime(tm);

    std::string Timestamp = getTimestampString();
    std::string Method = "DELETE";
    std::string requestPath = "/api/v1/order/all";
    string Message = Method + requestPath;
    Message += std::to_string(next);

    std::string signature = hmac_sha256(unit.secret_key.c_str(), Message.c_str());
    string url = unit.baseUrl + requestPath;

    const auto response = Delete(Url{url},
                                 Header{{"api-key", unit.api_key},
                                        {"Content-Type", "application/json"},
                                        {"api-signature", signature},
                                        {"api-expires", std::to_string(next) }}
                                 );

    KF_LOG_INFO(logger, "[cancel_all_orders] (url) " << url  << " (response.status_code) " << response.status_code <<
                                                     " (response.error.message) " << response.error.message <<
                                                     " (response.text) " << response.text.c_str());
    getResponse(response.status_code, response.text, response.error.message, json);
}


void TDEngineBitmex::cancel_order(AccountUnitBitmex& unit, long orderId, Document& json)
{
    KF_LOG_INFO(logger, "[cancel_order]");

    std::string Timestamp = getTimestampString();
    std::time_t baseNow = std::time(nullptr);
    struct tm* tm = std::localtime(&baseNow);
    tm->tm_sec += 30;
    std::time_t next = std::mktime(tm);

    std::string Method = "DELETE";
    std::string requestPath = "/api/v1/order";
    std::string queryString= "";
    std::string body = "clOrdID=" + std::to_string(orderId);

    string Message = Method + requestPath + queryString + body;
    std::string signature = hmac_sha256(unit.secret_key.c_str(), Message.c_str());

    string url = unit.baseUrl + requestPath + queryString;

    const auto response = Delete(Url{url},
                                 Header{{"api-key", unit.api_key},
                                        {"Content-Type", "application/json"},
                                        {"api-signature", signature},
                                        {"api-expires", std::to_string(next) }},
                                 Body{body}, Timeout{30000});

    KF_LOG_INFO(logger, "[cancel_order] (url) " << url  << " (body) "<< body << " (response.status_code) " << response.status_code <<
                                                " (response.error.message) " << response.error.message <<
                                                " (response.text) " << response.text.c_str());
    getResponse(response.status_code, response.text, response.error.message, json);
}



void TDEngineBitmex::query_order(AccountUnitBitmex& unit, std::string code, long orderId, Document& json)
{
    KF_LOG_INFO(logger, "[query_order]");
/*
 # Response
    {
        "averagePrice":"0",
        "code":"chp-eth",
        "createdDate":9887828,
        "filledVolume":"0",
        "funds":"0",
        "orderId":9865872,
        "orderType":"limit",
        "price":"0.00001",
        "side":"buy",
        "status":"canceled",
        "volume":"1"
    }
* */
    std::string Timestamp = getTimestampString();
    std::string Method = "GET";
    std::string requestPath = "/api/v1/order";
    std::string body = "";
    string Message = Method + requestPath + body;
    unsigned char* signature = hmac_sha256_byte(unit.secret_key.c_str(), Message.c_str());
    string url = unit.baseUrl + requestPath;
    std::string sign = base64_encode(signature, 32);
    const auto response = Get(Url{url},
                              Header{{"ACCESS-KEY", unit.api_key},
                                     {"Content-Type", "application/json"},
                                     {"ACCESS-SIGN", sign},
                                     {"ACCESS-TIMESTAMP",  Timestamp}},
                              Body{body}, Timeout{10000});

    KF_LOG_INFO(logger, "[query_order] (url) " << url << " (response.status_code) " << response.status_code <<
                                               " (response.error.message) " << response.error.message <<
                                               " (response.text) " << response.text.c_str());
    //(response.status_code) 404 (response.error.message)  (response.text) {"message":"Order does not exist"}
    getResponse(response.status_code, response.text, response.error.message, json);
}

inline int64_t TDEngineBitmex::getTimestamp()
{
    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return timestamp;
}

std::string TDEngineBitmex::getTimestampString()
{
    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    std::string timestampStr;
    std::stringstream convertStream;
    convertStream <<std::fixed << std::setprecision(3) << (timestamp/1000.0);
    convertStream >> timestampStr;
    return timestampStr;
}


#define GBK2UTF8(msg) kungfu::yijinjing::gbk2utf8(string(msg))

BOOST_PYTHON_MODULE(libbitmextd)
{
    using namespace boost::python;
    class_<TDEngineBitmex, boost::shared_ptr<TDEngineBitmex> >("Engine")
            .def(init<>())
            .def("init", &TDEngineBitmex::initialize)
            .def("start", &TDEngineBitmex::start)
            .def("stop", &TDEngineBitmex::stop)
            .def("logout", &TDEngineBitmex::logout)
            .def("wait_for_stop", &TDEngineBitmex::wait_for_stop);
}
