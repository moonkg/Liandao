#include "TDEngineCoinmex.h"
#include "TypeConvert.hpp"
#include "Timer.h"
#include "longfist/LFUtils.h"
#include "longfist/LFDataStruct.h"

#include <document.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <cpr/cpr.h>
#include <chrono>


USING_WC_NAMESPACE

TDEngineCoinmex::TDEngineCoinmex(): ITDEngine(SOURCE_COINMEX)
{
    logger = yijinjing::KfLog::getLogger("TradeEngine.Coinmex");
    KF_LOG_INFO(logger, "[ATTENTION] default to confirm settlement and no authentication!");
}

void TDEngineCoinmex::init()
{
    ITDEngine::init();
    JournalPair tdRawPair = getTdRawJournalPair(source_id);
    raw_writer = yijinjing::JournalWriter::create(tdRawPair.first, tdRawPair.second, "RAW_" + name());
}

void TDEngineCoinmex::pre_load(const json& j_config)
{
    KF_LOG_INFO(logger, "[pre_load]");
}

void TDEngineCoinmex::resize_accounts(int account_num)
{
    account_units.resize(account_num);
    KF_LOG_INFO(logger, "[resize_accounts]");
}

TradeAccount TDEngineCoinmex::load_account(int idx, const json& j_config)
{
    KF_LOG_INFO(logger, "[load_account]");
    // internal load
    string api_key = j_config["APIKey"].get<string>();
    string secret_key = j_config["SecretKey"].get<string>();
    rest_get_interval_ms = j_config["rest_get_interval_ms"].get<int>();
    /*
    //init all of the possible symbol?
    std::string define_symbols = j_config["symbols"].get<string>();
    KF_LOG_INFO(logger, "[load_account] (api_key)" << api_key << " (rest_get_interval_ms)" << rest_get_interval_ms << " (symbols)" << define_symbols);
    std::vector<std::string> fields = split(define_symbols, " ");
    for (size_t n = 0; n < fields.size(); n++)
    {
        
        symbols_pending_orderref.insert(std::pair<std::string, std::vector<std::string>*>(fields[n], new std::vector<std::string>()));
    }
    */
    AccountUnitCoinmex& unit = account_units[idx];
    unit.api_key = api_key;
    unit.secret_key = secret_key;

    // set up
    TradeAccount account = {};
    //partly copy this fields
    strncpy(account.UserID, api_key.c_str(), 16);
    strncpy(account.Password, secret_key.c_str(), 21);
    return account;
}

void TDEngineCoinmex::connect(long timeout_nsec)
{
    KF_LOG_INFO(logger, "[connect]");
    KF_LOG_INFO(logger, "[connect] rest_thread start on TDEngineCoinmex::loop");
    rest_thread = ThreadPtr(new std::thread(boost::bind(&TDEngineCoinmex::loop, this)));
}

void TDEngineCoinmex::login(long timeout_nsec)
{
    KF_LOG_INFO(logger, "[login]");
    connect(timeout_nsec);
}

void TDEngineCoinmex::logout()
{
    KF_LOG_INFO(logger, "[logout]");
}

void TDEngineCoinmex::release_api()
{
    KF_LOG_INFO(logger, "[release_api]");
}

bool TDEngineCoinmex::is_logged_in() const
{
    KF_LOG_INFO(logger, "[is_logged_in]");
    for (auto& unit: account_units)
    {
        if (!unit.logged_in)
            return false;
    }
    return true;
}

bool TDEngineCoinmex::is_connected() const
{
    KF_LOG_INFO(logger, "[is_connected]");
    return is_logged_in();
}



std::string TDEngineCoinmex::GetSide(const LfDirectionType& input) {
    if (LF_CHAR_Buy == input) {
        return "BUY";
    } else if (LF_CHAR_Sell == input) {
        return "SELL";
    } else {
        return "UNKNOWN";
    }
}

 LfDirectionType TDEngineCoinmex::GetDirection(std::string input) {
    if ("BUY" == input) {
        return LF_CHAR_Buy;
    } else if ("SELL" == input) {
        return LF_CHAR_Sell;
    } else {
        return LF_CHAR_Buy;
    }
}

std::string TDEngineCoinmex::GetType(const LfOrderPriceTypeType& input) {
    if (LF_CHAR_LimitPrice == input) {
        return "LIMIT";
    } else if (LF_CHAR_AnyPrice == input) {
        return "MARKET";
    } else {
        return "UNKNOWN";
    }
}

LfOrderPriceTypeType TDEngineCoinmex::GetPriceType(std::string input) {
    if ("LIMIT" == input) {
        return LF_CHAR_LimitPrice;
    } else if ("MARKET" == input) {
        return LF_CHAR_AnyPrice;
    } else {
        return '0';
    }
}

std::string TDEngineCoinmex::GetTimeInForce(const LfTimeConditionType& input) {
    if (LF_CHAR_IOC == input) {
      return "IOC";
    } else if (LF_CHAR_GFD == input) {
      return "GTC";
    } else if (LF_CHAR_FOK == input) {
      return "FOK";
    } else {
      return "UNKNOWN";
    }
}

LfTimeConditionType TDEngineCoinmex::GetTimeCondition(std::string input) {
    if ("IOC" == input) {
      return LF_CHAR_IOC;
    } else if ("GTC" == input) {
      return LF_CHAR_GFD;
    } else if ("FOK" == input) {
      return LF_CHAR_FOK;
    } else {
      return '0';
    }
}

LfOrderStatusType TDEngineCoinmex::GetOrderStatus(std::string input) {
    if ("NEW" == input) {
      return LF_CHAR_NotTouched;
    } else if ("PARTIALLY_FILLED" == input) {
      return LF_CHAR_PartTradedNotQueueing;
    } else if ("FILLED" == input) {
      return LF_CHAR_AllTraded;
    } else if ("CANCELED" == input) {
      return LF_CHAR_Canceled;
    } else if ("PENDING_CANCEL" == input) {
      return LF_CHAR_NotTouched;
    } else if ("REJECTED" == input) {
      return LF_CHAR_Error;
    } else if ("EXPIRED" == input) {
      return LF_CHAR_Error;
    } else {
      return LF_CHAR_AllTraded;
    }
}


std::string TDEngineCoinmex::GetInputOrderData(const LFInputOrderField* order, int recvWindow) {
    std::stringstream ss;
    ss << "symbol=" << order->InstrumentID << "&";
    ss << "side=" << GetSide(order->Direction) << "&";
    ss << "type=" << GetType(order->OrderPriceType) << "&";
    ss << "timeInForce=" << GetTimeInForce(order->TimeCondition) << "&";
    ss << "quantity=" << order->Volume << "&";
    ss << "price=" << order->LimitPrice << "&";
    ss << "recvWindow="<< recvWindow <<"&timestamp=" << yijinjing::getNanoTime();
    return ss.str();
}

/**
 * req functions
 */
void TDEngineCoinmex::req_investor_position(const LFQryPositionField* data, int account_index, int requestId)
{
    KF_LOG_INFO(logger, "[req_investor_position]");

    KF_LOG_DEBUG(logger, "[req_investor_position] (Mock EmptyData)" << " (Bid)" << data->BrokerID
                                     << " (Iid)" << data->InvestorID
                                     << " (Tid)" << data->InstrumentID);
    send_writer->write_frame(data, sizeof(LFQryPositionField), source_id, MSG_TYPE_LF_QRY_POS_COINMEX, 1, requestId);
    std::string msg = "";
    LFRspPositionField pos;
    strncpy(pos.BrokerID, data->BrokerID, 11);
    strncpy(pos.InvestorID, data->InvestorID, 19);
    strncpy(pos.InstrumentID, data->InstrumentID, 31);
    on_rsp_position(&pos, 1, requestId, 0, msg.c_str());
}

void TDEngineCoinmex::req_qry_account(const LFQryAccountField *data, int account_index, int requestId)
{
    KF_LOG_INFO(logger, "[req_qry_account]");
}


void TDEngineCoinmex::GetAndHandleTradeResponse(const std::string& symbol, int limit)
{
    //rhese codes is just for test
    std::vector<std::string> symbols;
    uint64_t last_trade_id = 0;
    const auto static url = "https://api.binance.com/api/v1/trades";
    const auto response = cpr::Get(cpr::Url{url}, cpr::Parameters{{"symbol", symbol},
                                                   {"limit",  std::to_string(limit)}});
    rapidjson::Document d;
    d.Parse(response.text.c_str());
    if(d.IsArray())
    {
        LFL2TradeField trade;
        memset(&trade, 0, sizeof(trade));
        strcpy(trade.InstrumentID, symbols[0].c_str());
        strcpy(trade.ExchangeID, "binance");

        for(int i = 0; i < d.Size(); ++i)
        {
            const auto& ele = d[i];
            if(!ele.HasMember("id"))
            {
                continue;
            }

            const auto trade_id = ele["id"].GetUint64();
            if(trade_id <= last_trade_id)
            {
                continue;
            }

            last_trade_id = trade_id;
            if(ele.HasMember("price") && ele.HasMember("qty") && ele.HasMember("isBuyerMaker") && ele.HasMember("isBestMatch"))
            {
                trade.Price = std::stod(ele["price"].GetString()) * scale_offset;
                trade.Volume = std::stod(ele["qty"].GetString()) * scale_offset;
                trade.OrderKind[0] = ele["isBestMatch"].GetBool() ? 'B' : 'N';
                trade.OrderBSFlag[0] = ele["isBuyerMaker"].GetBool() ? 'B' : 'S';

            }
        }
    }
}


void TDEngineCoinmex::req_order_insert(const LFInputOrderField* data, int account_index, int requestId, long rcv_time)
{

}

void TDEngineCoinmex::req_order_action(const LFOrderActionField* data, int account_index, int requestId, long rcv_time)
{

}

void TDEngineCoinmex::GetAndHandleOrderResponse()
{

}

void TDEngineCoinmex::loop()
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
        GetAndHandleOrderResponse();
    }
}



std::vector<std::string> TDEngineCoinmex::split(std::string str, std::string token)
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



#define GBK2UTF8(msg) kungfu::yijinjing::gbk2utf8(string(msg))

BOOST_PYTHON_MODULE(libcoinmextd)
{
    using namespace boost::python;
    class_<TDEngineCoinmex, boost::shared_ptr<TDEngineCoinmex> >("Engine")
    .def(init<>())
    .def("init", &TDEngineCoinmex::initialize)
    .def("start", &TDEngineCoinmex::start)
    .def("stop", &TDEngineCoinmex::stop)
    .def("logout", &TDEngineCoinmex::logout)
    .def("wait_for_stop", &TDEngineCoinmex::wait_for_stop);
}




