#include "TDEnginePoloniex.h"
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
#include <map>
#include <stdio.h>
#include <assert.h>
#include <mutex>
#include <chrono>
#include "../../utils/crypto/openssl_util.h"

using cpr::Delete;
using cpr::Url;
using cpr::Body;
using cpr::Header;
using cpr::Parameters;
using cpr::Payload;
using cpr::Timeout;

/*
using rapidjson::StringRef;
using rapidjson::Writer;
using rapidjson::StringBuffer;
using rapidjson::Document;
using rapidjson::SizeType;
using rapidjson::Value;
*/
using std::string;
using std::to_string;
using std::stod;
using std::stoi;
using std::stoll;
using utils::crypto::hmac_sha256;
using utils::crypto::hmac_sha256_byte;
using utils::crypto::base64_encode;
using utils::crypto::hmac_sha512;
USING_WC_NAMESPACE

#define NOT_TOUCHED 300
#define JUST_ERROR 400
#define PARSE_ERROR 401
#define EXEC_ERROR 402
#define PARA_ERROR 403
#define NOT_FOUND 404
#define ORDER_CLOSED 422
void TDEnginePoloniex::init()
{
    ITDEngine::init();
    JournalPair tdRawPair = getTdRawJournalPair(source_id);
    raw_writer = yijinjing::JournalSafeWriter::create(tdRawPair.first, tdRawPair.second, "RAW_" + name());
    KF_LOG_INFO(logger, "[init]");
}

void TDEnginePoloniex::pre_load(const json& j_config)
{
    //占位
    KF_LOG_INFO(logger, "[pre_load]");
}

TradeAccount TDEnginePoloniex::load_account(int idx, const json& j_config)
{
    KF_LOG_INFO(logger, "[load_account]");
    AccountUnitPoloniex& unit = account_units[idx];
    //加载必要参数
    string api_key = j_config["APIKey"].get<string>();
    string secret_key = j_config["SecretKey"].get<string>();
    string baseUrl = j_config["baseUrl"].get<string>();
	KF_LOG_INFO(logger, "[load_account] (baseUrl_private_point)" << baseUrl);
    //币对白名单设置
    unit.coinPairWhiteList.ReadWhiteLists(j_config, "whiteLists");
    unit.coinPairWhiteList.Debug_print();
    //持仓白名单设置
    unit.positionWhiteList.ReadWhiteLists(j_config, "positionWhiteLists");
    unit.positionWhiteList.Debug_print();

    //可选配置参数如下设立
    if (j_config.find("retry_interval_milliseconds") != j_config.end()) {
        retry_interval_milliseconds = j_config["retry_interval_milliseconds"].get<int>();
    }
	else
	{
		retry_interval_milliseconds = 1000;
	}

	if (j_config.find("max_retry_times") != j_config.end()) {
		max_retry_times = j_config["max_retry_times"].get<int>();
	}
	else
	{
		max_retry_times = 5;
	}
    KF_LOG_INFO(logger, "[load_account] (max_retry_times)" << max_retry_times);

	if (j_config.find("url_public_point") != j_config.end()) {
		url_public_point = j_config["url_public_point"].get<int>();
	}
	else
	{
		url_public_point = "https://poloniex.com/public";
	}
	KF_LOG_INFO(logger, "[load_account] (url_public_point)" << url_public_point);

    //账户设置
    unit.api_key = api_key;
    unit.secret_key = secret_key;
    unit.baseUrl = baseUrl;
    unit.positionHolder.clear();

    //系统账户信息
    TradeAccount account = {};
    strncpy(account.UserID, api_key.c_str(), 16);
    strncpy(account.Password, secret_key.c_str(), 21);
    //simply for rest api test
    /*
    string timestamp = to_string(get_timestamp());
    string command = "command=returnBalances&nonce="+timestamp;
    string method = "POST";
	KF_LOG_DEBUG(logger, "[getbalance]" );
    cpr::Response r = rest_withAuth(unit, method, command);//获得账户余额消息
	
	string currencyPair = "BTC_ETH";
	return_orderbook(currencyPair, 1);//测试一下，，，
	*/
    //test ends here
	KF_LOG_INFO(logger, "[load_account] updating order status thread starts");
	isRunning = 1;
	rest_thread = ThreadPtr(new std::thread(boost::bind(&TDEnginePoloniex::updating_order_status, this)));

    return account;
}

void TDEnginePoloniex::resize_accounts(int account_num)
{
    //占位
    account_units.resize(account_num);
    KF_LOG_INFO(logger, "[resize_accounts]");
}

void TDEnginePoloniex::connect(long timeout_nsec)
{
    //占位
    KF_LOG_INFO(logger, "[connect]");
    for (auto& unit : account_units)
    {
        //以后可以通过一个需认证的接口访问是否成功来判断是否有正确的密钥对，成功则登入，目前没有需要
        unit.logged_in = true;//maybe we need to add some "if" here
    }
}

void TDEnginePoloniex::login(long timeout_nsec)
{
    //占位
    KF_LOG_INFO(logger, "[login]");
    connect(timeout_nsec);
}

void TDEnginePoloniex::logout()
{
    //占位
    KF_LOG_INFO(logger, "[logout]");
}

void TDEnginePoloniex::release_api()
{
    //占位
    KF_LOG_INFO(logger, "[release_api]");
}

bool TDEnginePoloniex::is_connected() const
{
    KF_LOG_INFO(logger, "[is_connected]");
    return is_logged_in();
}

bool TDEnginePoloniex::is_logged_in() const
{
    //占位
    KF_LOG_INFO(logger, "[is_logged_in]");
    /*for(auto &unit:account_units)  //启用多用户后，此处需修改
      {
      if(!unit.logged_in)
      return false;
      }*/
    return true;
}

void TDEnginePoloniex::req_investor_position(const LFQryPositionField* data, int account_index, int requestId)
{
    KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId);
    AccountUnitPoloniex& unit = account_units[account_index];
    KF_LOG_INFO(logger, "[req_investor_position] (InstrumentID) " << data->InstrumentID);
    send_writer->write_frame(data, sizeof(LFQryPositionField), source_id, MSG_TYPE_LF_QRY_POS_POLONIEX, 1, requestId);
    int errorId = 0;
    std::string errorMsg = "";

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

    /*实现一个函数获得balance信息，一个函数解析并存储到positionHolder里面去*/
	string command = "command=returnBalances&nonce=";
    string method = "POST";
	int count = 1;
	KF_LOG_DEBUG(logger, "[getbalance]" );
    cpr::Response r = rest_withAuth(unit, method, command);//获得账户余额消息
	KF_LOG_INFO(logger, " (command) " << command <<
		" (response.status_code) " << r.status_code <<
		" (response.error.message) " << r.error.message <<
		" (response.text) " << r.text.c_str());
    json js;
    while (true)
    {
        if (r.status_code == 200)//获得余额信息的操作成功了
        {
			int ret = get_response_parsed_position(r);
            if (ret==0)
            {
                /*解析response并存入unit.positionHolder*/
                /*若是解析成功则返回1*/
                break;
            }
            else if(ret==1)
            {
				errorId = PARSE_ERROR;
				errorMsg = "req investor position response parsed error,it's not an object, quit";
				KF_LOG_ERROR(logger, errorId);
				raw_writer->write_error_frame(&pos, sizeof(LFRspPositionField), 
                        source_id, MSG_TYPE_LF_RSP_POS_POLONIEX, 1, requestId, errorId, errorMsg.c_str());
				return;
            }
			else if(ret==2)
			{
				errorId = JUST_ERROR;
				errorMsg = r.text;
				KF_LOG_ERROR(logger, "req investor position "<<r.text<<" quit");
				raw_writer->write_error_frame(&pos, sizeof(LFRspPositionField), 
                        source_id, MSG_TYPE_LF_RSP_POS_POLONIEX, 1, requestId, errorId, errorMsg.c_str());
				return;
			}
        }
        else
        {
			count++;
			if (count > max_retry_times)
			{
				errorMsg = "after several retry,req investor position still failed";
				KF_LOG_ERROR(logger, errorMsg);
				errorId = EXEC_ERROR;
				raw_writer->write_error_frame(&pos, sizeof(LFRspPositionField), 
                        source_id, MSG_TYPE_LF_RSP_POS_POLONIEX, 1, requestId, errorId, errorMsg.c_str());
				on_rsp_position(&pos, 1, requestId, errorId, errorMsg.c_str());
				return;
			}
            KF_LOG_ERROR(logger, "req investor position failed,retry after retry_interval_milliseconds");
			std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
			KF_LOG_DEBUG(logger, "[req_investor_position]");
            r = rest_withAuth(unit, method, command);
			KF_LOG_INFO(logger, " (command) " << command <<
				" (response.status_code) " << r.status_code <<
				" (response.error.message) " << r.error.message <<
				" (response.text) " << r.text.c_str());
        }
    }
    bool findSymbolInResult = false;
    //send the filtered position
    int position_count = unit.positionHolder.size();
    for (int i = 0; i < position_count; i++)//逐个将获得的ticker存入pos
    {
        strncpy(pos.InstrumentID, unit.positionHolder[i].ticker.c_str(), 31);
        if (unit.positionHolder[i].isLong) {
            pos.PosiDirection = LF_CHAR_Long;
        }
        else {
            pos.PosiDirection = LF_CHAR_Short;
        }
        pos.Position = unit.positionHolder[i].amount;
        on_rsp_position(&pos, i == (position_count - 1), requestId, 0, errorMsg.c_str());
        findSymbolInResult = true;
    }

    if (!findSymbolInResult)
    {
        KF_LOG_INFO(logger, "[req_investor_position] (!findSymbolInResult) (requestId)" << requestId);
		errorId = NOT_FOUND;
		errorMsg = "!findSymbolInResult";
        on_rsp_position(&pos, 1, requestId, errorId, errorMsg.c_str());
    }
}

void TDEnginePoloniex::req_qry_account(const LFQryAccountField* data, int account_index, int requestId)
{
    KF_LOG_INFO(logger, "[req_qry_account]");
}

void TDEnginePoloniex::req_order_insert(const LFInputOrderField* data, int account_index, int requestId, long rcv_time)
{
    KF_LOG_INFO(logger, "[req_order_insert]");
	send_writer->write_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER_POLONIEX, 1, requestId);
	on_rsp_order_insert(data, requestId, 0, "");

	AccountUnitPoloniex& unit = account_units[0];
	int errorId = 0;
	string errorMsg="";
	string currency_pair = unit.coinPairWhiteList.GetValueByKey(string(data->InstrumentID));
	string order_type = get_order_type(data->OrderPriceType);//市价单或限价单
	string order_side = get_order_side(data->Direction);//买或者卖
	double rate = data->LimitPrice * 1.0 / scale_offset;
	string rate_str = to_string(rate);
	double amount = data->Volume * 1.0 / scale_offset;
	string amount_str = to_string(amount);
	string timestamp = to_string(get_timestamp());
	//"command=buy&currencyPair=BTC_ETH&rate=0.01&amount=1&nonce=154264078495300"
	string command = "command=";
	if (data->OrderPriceType == LF_CHAR_LimitPrice)//说明是限价单
	{
		command += order_side +
			"&currencyPair=" + currency_pair +
			"&rate=" + rate_str +
			"&amount=" + amount_str +
			"&nonce=";
	}
	else
	{
		//若是市价单，，暂时还没有办法处理，先出错处理吧
		//可能新建一个线程来处理？暂时先放着，日后可能有其它办法了再说
		KF_LOG_ERROR(logger, "[req_order_insert] (market order error) ");
		KF_LOG_ERROR(logger, "[req_order_insert] (please use limitprice order instead) ");
		errorId = EXEC_ERROR;
		errorMsg = "market order error";
		on_rsp_order_insert(data, requestId, errorId, errorMsg.c_str());
		return;
	}
	string parastring="";
	if (is_post_only(data)) parastring += "&postOnly=1";
	if (data->TimeCondition == LF_CHAR_FAK) parastring += "&immediateOrCancel=1";
	if (data->TimeCondition == LF_CHAR_FOK) parastring += "&fillOrKill=1";
	OrderInfo order_info;
	order_info.timestamp = timestamp;//TODO:这个可能要去掉，并非实际发单时间
	order_info.currency_pair = currency_pair;
	order_info.volume_total_original = data->Volume;
	cpr::Response r;
	json js;
	int count = 1;
	string method = "POST";
	string fullcommand = command + parastring;
	KF_LOG_INFO(logger, "[req order insert]");
	r = rest_withAuth(unit, method, fullcommand);
	KF_LOG_INFO(logger, " (command) " << command <<
		" (response.status_code) " << r.status_code <<
		" (response.error.message) " << r.error.message <<
		" (response.text) " << r.text.c_str());
	//发单错误或者异常状况处理
	while (true)
	{
		if (r.status_code == 200)//操作发送成功，但不代表操作生效，可能有参数错误等
		{
			js = json::parse(r.text);//解析
			if (js.is_object())
			{
				if (js.find("error") != js.end() || js.find("orderNumber") == js.end())//错误回报，或者回报中没有orderNumber（可省略）
				{
					//出错处理，此种情况一般为参数错误，，，需要修改参数
					KF_LOG_ERROR(logger, "[req_order_insert] (insert order error) "<<r.text);
					errorId = PARA_ERROR;
					errorMsg = r.text;
					raw_writer->write_error_frame(data, sizeof(LFInputOrderField),
						source_id, MSG_TYPE_LF_RSP_POS_POLONIEX, 1, requestId, errorId, errorMsg.c_str());
					return;
				}
				order_info.order_number = stoll(js["orderNumber"].get<string>());
				break;
			}
		}
		else
		{
			count++;
			if (count > max_retry_times)
			{
				errorMsg = "after several retry,get balance still failed";
				KF_LOG_ERROR(logger, errorMsg<<"(count)"<<count);
				errorId = EXEC_ERROR;
				raw_writer->write_error_frame(data, sizeof(LFInputOrderField),
					source_id, MSG_TYPE_LF_RSP_POS_POLONIEX, 1, requestId, errorId, errorMsg.c_str());
				return;
			}
			KF_LOG_ERROR(logger, "req order insert failed,retry after retry_interval_milliseconds");
			std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
			KF_LOG_DEBUG(logger, "[req_order_insert]");
			r = rest_withAuth(unit, method, fullcommand);
			KF_LOG_INFO(logger, " (command) " << command <<
				" (response.status_code) " << r.status_code <<
				" (response.error.message) " << r.error.message <<
				" (response.text) " << r.text.c_str());
		}
	}
	//获得订单信息，处理 order_info、rtn_order
	string order_ref = string(data->OrderRef);
	order_info.order_ref = order_ref;
	order_info.tradeID = 0;
	order_info.is_open = true;

	std::unique_lock<std::mutex> lock_map_new_order(*mutex_new_order);
	unit.map_new_order.insert(std::make_pair(order_ref, order_info));
	lock_map_new_order.unlock();

	LFRtnOrderField rtn_order;//返回order信息
	memset(&rtn_order, 0, sizeof(LFRtnOrderField));
	string order_number_str = to_string(order_info.order_number);
	//以下为必填项
    strncpy(rtn_order.OrderRef, data->OrderRef, 21);
	strncpy(rtn_order.BusinessUnit, order_number_str.c_str(), order_number_str.length());
	rtn_order.OrderStatus = LF_CHAR_NotTouched;
	rtn_order.LimitPrice = data->LimitPrice;
	rtn_order.VolumeTotalOriginal = data->Volume;
	rtn_order.VolumeTraded = 0;//刚刚发单，看作没有成交，等后面更新订单状态时再逐渐写入
	rtn_order.VolumeTotal = rtn_order.VolumeTotalOriginal - rtn_order.VolumeTraded;
	//其余的为可填项，尽量补全
	strncpy(rtn_order.UserID, data->UserID, 16);
	strncpy(rtn_order.InstrumentID, data->InstrumentID, 31);
	strncpy(rtn_order.ExchangeID, "poloniex", 8);
	rtn_order.RequestID = requestId;
	rtn_order.Direction = data->Direction;
	rtn_order.TimeCondition = data->TimeCondition;
	rtn_order.OrderPriceType = data->OrderPriceType;
	on_rtn_order(&rtn_order);
	//插入map，方便后续订单状态跟踪
	std::unique_lock<std::mutex> lock_map_order(*mutex_order);
	map_order.insert(std::make_pair(order_ref, rtn_order));
	lock_map_order.unlock();
}

void TDEnginePoloniex::req_order_action(const LFOrderActionField* data, int account_index, int requestId, long rcv_time)
{
	KF_LOG_DEBUG(logger, "[req_order_action]" << " (rid) " << requestId);
	send_writer->write_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_POLONIEX, 1, requestId);
	AccountUnitPoloniex& unit = account_units[0];
	//get order status
	cpr::Response r;
	int errorId=0;
	string errorMsg = "";
	string order_ref = string(data->OrderRef);
	OrderInfo order_info;
	std::unique_lock<std::mutex> lock_map_new_order(*mutex_new_order);
	if (unit.map_new_order.count(order_ref))
	{
		order_info = unit.map_new_order[order_ref];
	}
	else
	{
		//出错处理
		errorMsg = "couldn't find this order by OrderRef,it might be cancelled or completed";
		errorId = NOT_FOUND;
		on_rsp_order_action(data, requestId, errorId, errorMsg.c_str());
		raw_writer->write_error_frame(data, sizeof(LFOrderActionField),
			source_id, MSG_TYPE_LF_RSP_POS_POLONIEX, 1, requestId, errorId, errorMsg.c_str());
		return ;
	}
	lock_map_new_order.unlock();
	string method = "POST";
	string command = "command=cancelOrder&orderNumber=" + to_string(order_info.order_number) +
		"&nonce=";
	KF_LOG_INFO(logger, "[req order action]");
	r=rest_withAuth(unit, method, command);
	KF_LOG_INFO(logger, " (command) " << command <<
		" (response.status_code) " << r.status_code <<
		" (response.error.message) " << r.error.message <<
		" (response.text) " << r.text.c_str());
	//出错及异常处理
	//需要特别注意单订单不存在或者已经成交了的话会返回错误码422,如果是操作的nonce错了也会返回422
	json js;
	int count;
	while (true)
	{
		if (r.status_code == 200||r.status_code==422)//操作发送成功，但不代表操作生效，可能有参数错误等,或者订单已经被撤销了
		{
			js = json::parse(r.text);//解析
			if (js.is_object())
			{
				if (js.find("success") != js.end())
				{
					KF_LOG_INFO(logger, "[req_order_action] (order cancelled) ");
					break;
				}
				if (js.find("error") != js.end())
				{
					//出错处理，此种情况一般为参数错误，，，需要修改参数
					errorId = PARA_ERROR;
					KF_LOG_ERROR(logger, "[req_order_action] (cancel error) " << r.text);
					raw_writer->write_error_frame(&data, sizeof(LFOrderActionField),
						source_id, MSG_TYPE_LF_RSP_POS_POLONIEX, 1, requestId, errorId, errorMsg.c_str());
					break;
				}
			}
			KF_LOG_ERROR(logger, "[req_order_action] (cancel error) " << r.text);
			raw_writer->write_error_frame(&data, sizeof(LFOrderActionField),
				source_id, MSG_TYPE_LF_RSP_POS_POLONIEX, 1, requestId, errorId, errorMsg.c_str());
		}
		else
		{
			count++;
			if (count > max_retry_times)
			{
				errorMsg = "after several retry,req order action still failed";
				KF_LOG_ERROR(logger, errorMsg << "(count)" << count);
				errorId = EXEC_ERROR;
				raw_writer->write_error_frame(data, sizeof(LFOrderActionField),
					source_id, MSG_TYPE_LF_RSP_POS_POLONIEX, 1, requestId, errorId, errorMsg.c_str());
				break;
			}
			KF_LOG_ERROR(logger, "req order action failed,retry after retry_interval_milliseconds");
			std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
			KF_LOG_DEBUG(logger, "[req_order_action]");
			r = rest_withAuth(unit, method, command);
			KF_LOG_INFO(logger, " (command) " << command <<
				" (response.status_code) " << r.status_code <<
				" (response.error.message) " << r.error.message <<
				" (response.text) " << r.text.c_str());
		}
	}
	
	on_rsp_order_action(data, requestId, errorId, errorMsg.c_str());
}

TDEnginePoloniex::TDEnginePoloniex():ITDEngine(SOURCE_POLONIEX)
{
    logger = yijinjing::KfLog::getLogger("TradeEngine.Poloniex");
    KF_LOG_INFO(logger, "[TDEnginePoloniex]");

    mutex_order_and_trade = new std::mutex();
    mutex_response_order_status = new std::mutex();
    mutex_orderaction_waiting_response = new std::mutex();
	mutex_new_order = new std::mutex();
	mutex_order = new std::mutex();
}

TDEnginePoloniex::~TDEnginePoloniex()
{
    if (mutex_order_and_trade != nullptr) delete mutex_order_and_trade;
    if (mutex_response_order_status != nullptr) delete mutex_response_order_status;
    if (mutex_orderaction_waiting_response != nullptr) delete mutex_orderaction_waiting_response;
	if (mutex_new_order != nullptr) delete mutex_new_order;
	if (mutex_order != nullptr) delete mutex_order;
}

inline int64_t TDEnginePoloniex::get_timestamp()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

string TDEnginePoloniex::get_order_type(LfOrderPriceTypeType type)
{
    if (type == LF_CHAR_LimitPrice)
    {
        return "LIMIT";
    }
    else if (type == LF_CHAR_AnyPrice)
    {
        return "MARKET";
    }
    return "false";
}

string TDEnginePoloniex::get_order_side(LfDirectionType type)
{
    if (type == LF_CHAR_Buy)
    {
        return "buy";
    }
    else if (type == LF_CHAR_Sell)
    {
        return "sell";
    }
    return "false";
}

int TDEnginePoloniex::get_response_parsed_position(cpr::Response r)
{
    auto js = json::parse(r.text);
    AccountUnitPoloniex& unit = account_units[0];
    PositionSetting ps;
    if (js.is_object())
    {
		if (js.find("error") != js.end())
		{
			return 2;
		}
		std::unique_lock<std::mutex> lock(*mutex_order_and_trade);
        std::unordered_map<std::string, std::string>& whitelist=unit.positionWhiteList.GetKeyIsStrategyCoinpairWhiteList();
        std::unordered_map<std::string, std::string>::iterator it;
        for (it = whitelist.begin(); it != whitelist.end(); ++it)
        {
            string exchange_coinpair = it->first;
            ps.ticker = exchange_coinpair;
            ps.amount = stod(js[exchange_coinpair].get<string>());
            if (ps.amount > 0)
                ps.isLong = true;
            else ps.isLong = false;
            unit.positionHolder.push_back(ps);
        }
        return 0;
    }

    return 1;
}

cpr::Response TDEnginePoloniex::rest_withoutAuth(string& method, string& command)
{
    string Timestamp = to_string(get_timestamp());
    string url = url_public_point;
    url += "?"+command;
    // command= "command = returnOrderBook & currencyPair = BTC_ETH & depth = 10";
    cpr::Response response;
    if (!strcmp(method.c_str(), "GET"))
    {
        response = cpr::Get(
            Url{ url },
            Timeout{ 10000 }
        );
    }
    else if (!strcmp(method.c_str(), "POST"))
    {
        response = cpr::Post(
            Url{ url },
            Timeout{ 10000 }
        );
    }
    else
    {
        KF_LOG_ERROR(logger, "request method error");
        response.error.message = "request method error";
        response.status_code = 404;
        return response;
    }
    /*KF_LOG_INFO(logger, "[" << method << "] (url) " << url <<
        " (timestamp) " << Timestamp <<
        " (response.status_code) " << response.status_code <<
        " (response.error.message) " << response.error.message <<
        " (response.text) " << response.text.c_str());*/
    return response;
}

cpr::Response TDEnginePoloniex::rest_withAuth(AccountUnitPoloniex& unit, string& method, string& command)
{
    string key = unit.api_key;
    string secret = unit.secret_key;
    //url="https://poloniex.com/tradingApi";
    string url = unit.baseUrl;
    //command="command=returnBalances&nonce=154264078495300";
	string timestamp = to_string(get_timestamp());
    string body = command+timestamp;
    string sign=hmac_sha512(secret.c_str(),secret.length(),body.c_str(),body.length());
    cpr::Response response;
    if (!strcmp(method.c_str(), "GET"))
    {
        response = cpr::Get(
            Url{ url },
            Body{ body },
            Header{
            {"Key", key.c_str()},
            {"Sign",sign.c_str()}
            },
            Timeout{ 10000 }
        );
    }
    else if (!strcmp(method.c_str(), "POST"))
    {
        response = cpr::Post(
            Url{ url },
            Body{ body },
            Header{
            {"Key", key.c_str()},
            {"Sign",sign.c_str()}
            },
            Timeout{ 10000 }
        );
    }
    else
    {
        KF_LOG_ERROR(logger, "request method error");
        response.error.message = "request method error";
        response.status_code = 404;
        return response;
    }
    /*KF_LOG_INFO(logger, "[" << method << "] (url) " << url <<
        " (command) " << command <<
        //" (key) "<<key<<
        //" (secret) "<<secret<<
        " (sign) " << sign <<
        " (response.status_code) " << response.status_code <<
        " (response.error.message) " << response.error.message <<
        " (response.text) " << response.text.c_str());*/
    return response;
}

cpr::Response TDEnginePoloniex::return_orderbook(string& currency_pair,int level)
{
	/*
	(response.text) {"asks":[["0.03156507",3.4012095]],"bids":[["0.03156001",4.28179236]],"isFrozen":"0","seq":693326150}
	*/
	string method = "GET";
	string level_str = to_string(level);
	string command = "command=returnOrderBook&currencyPair="+currency_pair+
		"&depth="+level_str;
	cpr::Response r = rest_withoutAuth(method,command);
	KF_LOG_INFO(logger, "[return orderbook]");
	KF_LOG_INFO(logger, " (response.status_code) " << r.status_code <<
		" (response.error.message) " << r.error.message <<
		" (response.text) " << r.text.c_str());
	return r;
}

cpr::Response TDEnginePoloniex::return_order_status(int64_t& order_number)
{
	//KF_LOG_INFO(logger, "[return_order_status](order_number)" << order_number);
	AccountUnitPoloniex& unit = account_units[0];
	cpr::Response r;
	string method = "POST";
	string command = "command=returnOrderStatus&orderNumber=";
	string order_number_str = to_string(order_number);
	command += order_number_str +
		"&nonce=";
	//KF_LOG_INFO(logger, "[return order status]");
	r = rest_withAuth(unit, method, command);
	/*KF_LOG_INFO(logger, " (command) " << command <<
		" (response.status_code) " << r.status_code <<
		" (response.error.message) " << r.error.message <<
		" (response.text) " << r.text.c_str());*/
	//出错处理
	int count;
	string errorMsg = "";
	int errorId = 0;
	int success = 0;
	json js;
	while (true)
	{
		if (r.status_code == 200)//操作发送成功，但不代表操作生效，可能有参数错误等
		{
			js = json::parse(r.text);//解析
			if (js.is_object())//进一步进行错误判断，回报200仍可能是有错误的
			{
				//有可能success==0就表示找不到订单，不需要看后面的result
				if (js.find("success")!=js.end())
				{
					success = js["success"].get<int>();
				}
				if (js.find("result") != js.end())
				{
					auto result = js["result"];
					if (result.find("error") != result.end())
					{
						if (success == 0)//目前已知这种情况下为order not found,通常是因为订单完成或者撤单了
						{
							r.status_code = ORDER_CLOSED;
						}
						else
						{
							//出错处理，此种情况一般为参数错误，，，需要修改参数
							KF_LOG_ERROR(logger, "[return_order_status] " << r.text);
							r.status_code = PARA_ERROR;
						}
					}
					if (result.find(to_string(order_number)) != result.end())
					{
						auto order = result[to_string(order_number)];
						if (order.find("status") != order.end());
						{
							string status = order["status"].get<string>();
							if (strcmp("Open", status.c_str()) == 0)
							{
								r.status_code = NOT_TOUCHED;
							}
						}
					}
				}
				break;
			}
			else
			{
				//返回的不是正常的格式，可能存在未知错误
				KF_LOG_ERROR(logger, "[return_order_status] (it's not a object) " << r.text);
				r.status_code = PARSE_ERROR;
				break;
			}
		}
		else if (r.status_code == 422)//已知这个码在这里表示nonce错误了
		{
			KF_LOG_DEBUG(logger, "[return_order_status] (text) " << r.text);
			r.status_code = PARA_ERROR;
			break;
		}
		else//超时，或者网络状况不好等等错误
		{
			count++;
			if (count > max_retry_times)
			{
				errorMsg = "after several retry,return order status still failed";
				KF_LOG_ERROR(logger, errorMsg << "(count)" << count);
				r.status_code = EXEC_ERROR;
				break;
			}
			KF_LOG_ERROR(logger, "return order status failed,retry after retry_interval_milliseconds");
			std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
			//KF_LOG_DEBUG(logger, "[return_order_status]");
			r = rest_withAuth(unit, method, command);
			/*KF_LOG_INFO(logger, " (command) " << command <<
				" (response.status_code) " << r.status_code <<
				" (response.error.message) " << r.error.message <<
				" (response.text) " << r.text.c_str());*/
		}
	}
	return r;
}


cpr::Response TDEnginePoloniex::return_order_trades(int64_t& order_number)
{
	//KF_LOG_INFO(logger, "[return_order_trades](order_number)" << order_number);
	AccountUnitPoloniex& unit = account_units[0];
	cpr::Response r;
	string method = "POST";
	//string timestamp = to_string(get_timestamp());
	string command = "command=returnOrderTrades&orderNumber=";
	string order_number_str = to_string(order_number);
	command += order_number_str +"&nonce=";
	//KF_LOG_INFO(logger, "[return order trades]");
	r = rest_withAuth(unit, method, command);
	/*KF_LOG_INFO(logger, " (command) " << command <<
		" (response.status_code) " << r.status_code <<
		" (response.error.message) " << r.error.message <<
		" (response.text) " << r.text.c_str());*/
	//出错处理
	int count;
	string errorMsg = "";
	int errorId = 0;
	json js;
	while (true)
	{
		if (r.status_code == 200)//操作发送成功，但不代表操作生效，可能有参数错误等
		{
			js = json::parse(r.text);//解析
			if (js.is_object())
			{
				if (js.find("error") != js.end())//错误回报
				{
					//出错处理，此种情况一般为参数错误，，，需要修改参数
					KF_LOG_ERROR(logger, "[return_order_trades] " << r.text);
					r.status_code = PARA_ERROR;//TODO需要额外考虑
				}
				break;
			}
			else if (js.is_array())//正常情况返回一个数组
			{
				break;
			}
			else
			{
				KF_LOG_ERROR(logger, "[return_order_trades] (it can't be parsed) " << r.text);
				r.status_code = PARSE_ERROR;
				break;
			}
		}
		else
		{
			count++;
			if (count > max_retry_times)
			{
				errorMsg = "after several retry,return_order_trades still failed";
				KF_LOG_ERROR(logger, errorMsg << "(count)" << count);
				errorId = EXEC_ERROR;
				break;
			}
			KF_LOG_ERROR(logger, "[return_order_trades] failed,retry after retry_interval_milliseconds");
			std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
			//KF_LOG_DEBUG(logger, "[return_order_trades]");
			r = rest_withAuth(unit, method, command);
			/*KF_LOG_INFO(logger, " (command) " << command <<
				" (response.status_code) " << r.status_code <<
				" (response.error.message) " << r.error.message <<
				" (response.text) " << r.text.c_str());*/
		}
	}
	return r;
}

void TDEnginePoloniex::updating_order_status()
{
	KF_LOG_INFO(logger, "[updating_order_status] thread starts");
	bool is_closed = false;
	bool is_touched = false;
	//bool is_fully_open = false;
	AccountUnitPoloniex& unit = account_units[0];
	cpr::Response ro;
	cpr::Response rt;
	json js;
	LFRtnTradeField rtn_trade;
	while (isRunning)//这将是个死循环，时刻准备更新订单状态
	{
		std::unique_lock<std::mutex> lock_map_order(*mutex_order);
		if (map_order.size() == 0)
		{
			KF_LOG_INFO(logger, "[updating_order_status] looping!");
		}
		else
		{
			//KF_LOG_INFO(logger, "[updating_order_status] (map_order.size)" << map_order.size());
			map<string, LFRtnOrderField>::iterator it;
			for (it = map_order.begin(); it != map_order.end();)//遍历 map
			{
				LFRtnOrderField& rtn_order = it->second;//获得rtn order
				string order_ref = it->first;
				std::unique_lock<std::mutex> lock_map_new_order(*mutex_new_order);
				if (!unit.map_new_order.count(order_ref))
				{
					KF_LOG_ERROR(logger, "[updating_order_status] no order refed by order_ref" << order_ref);
					map_order.erase(order_ref);
					unit.map_new_order.erase(order_ref);
				}
				else
				{	//填写 rtn_order rtn_trade的部分共性信息

					OrderInfo& order_info = unit.map_new_order[order_ref];
					//KF_LOG_DEBUG(logger, "[updating_order_status] (order_ref) " << order_ref);
					memset(&rtn_trade, 0, sizeof(LFRtnTradeField));
					strcpy(rtn_trade.ExchangeID, "poloniex");
					strncpy(rtn_trade.UserID, rtn_order.UserID, 16);
					strncpy(rtn_trade.InstrumentID, rtn_order.InstrumentID, 31);
					strncpy(rtn_trade.OrderRef, rtn_order.OrderRef, 21);
					rtn_trade.Direction = rtn_order.Direction;

					int64_t tradeID = 0, tradeID_last;
					tradeID_last = order_info.tradeID;//上次更新到这个tradeID

					//先查order status，如果订单还在，查trades，如果订单不在，最后一次查trades
					is_closed = false;
					is_touched = false;
					//is_fully_open = false;
					ro = return_order_status(order_info.order_number);
					if (ro.status_code == ORDER_CLOSED)
					{
						is_closed = true;
					}
					else if (ro.status_code == 200)
					{
						//正常，不做处理
					}
					else if (ro.status_code >= 400)
					{
						//这个订单有问题，跳过
						KF_LOG_ERROR(logger, "[updating_order_status] error there are para problem or unknown mistake" <<
							" (order_number) " << order_info.order_number);
						it++;
						continue;
					}
					else if (ro.status_code==NOT_TOUCHED)
					{
						it++;
						continue;
					}
					rt = return_order_trades(order_info.order_number);
					/*if (!is_closed && rt.status_code == PARA_ERROR)//订单未关闭，却报错，且不是参数问题的话（通常不是，即使是也可以看log看出来）说明此单未有任何变化
					{
						it++;
						continue;
					}
					else */
					if (is_closed && rt.status_code == PARA_ERROR)//订单关闭，未有任何交易记录，被撤单了
					{
						order_info.is_open = false;
						rtn_order.OrderStatus = LF_CHAR_Canceled;
						on_rtn_order(&rtn_order);
					}
					else if (rt.status_code == 200)//有交易记录
					{
						js = json::parse(rt.text);
						int no = js.size();
						no--;
						while (no >= 0)//返回每一个trade，并返回order
						{
							auto ob = js[no];//获得这个object
							tradeID = ob["globalTradeID"].get<int64_t>();//int64_t
							if (tradeID > tradeID_last)//如果不是更新的trade就放弃
							{
								tradeID_last = tradeID;
								order_info.tradeID = tradeID;
								strcpy(rtn_trade.TradeID, std::to_string(tradeID).c_str());
								string price = to_string(stod(ob["rate"].get<string>()) * scale_offset);
								rtn_trade.Price = stoll(price);
								string volume = to_string(stod(ob["amount"].get<string>()) * scale_offset);
								rtn_trade.Volume = stoll(volume);
								on_rtn_trade(&rtn_trade);
								rtn_order.VolumeTraded += rtn_trade.Volume;
								rtn_order.VolumeTotal -= rtn_trade.Volume;
								if (rtn_order.VolumeTotal <= 0)//判断数量
								{
									rtn_order.OrderStatus = LF_CHAR_AllTraded;//已经完全成交了
									order_info.is_open = false;
									is_closed = true;
									//该订单处理结束							
									on_rtn_order(&rtn_order);
									break;
								}
								else
								{
									rtn_order.OrderStatus = LF_CHAR_PartTradedQueueing;
								}
								on_rtn_order(&rtn_order);
							}
							no--;
						}
						if (is_closed && order_info.is_open)//在撤单情况下，要再返回一次最终状态
						{
							order_info.is_open = false;
							rtn_order.OrderStatus = LF_CHAR_Canceled;
							on_rtn_order(&rtn_order);
						}
					}
					if (is_closed)//订单已经关闭了就要从order里剃出去
					{
						unit.map_new_order.erase(order_ref);
						map_order.erase(it++);
						KF_LOG_DEBUG(logger, "[updating_order_status] (order_ref) " << order_ref << " done ");
					}
					else
					{
						it++;
					}
					//要在这里结束一个订单的所有操作，接下来要开始下一个订单的状态更新和返回了
				}
				lock_map_new_order.unlock();
			}
		}
		lock_map_order.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
	}
}

BOOST_PYTHON_MODULE(libpoloniextd)
{
    using namespace boost::python;
    class_<TDEnginePoloniex, boost::shared_ptr<TDEnginePoloniex> >("Engine")
        .def(init<>())
        .def("init", &TDEnginePoloniex::initialize)
        .def("start", &TDEnginePoloniex::start)
        .def("stop", &TDEnginePoloniex::stop)
        .def("logout", &TDEnginePoloniex::logout)
        .def("wait_for_stop", &TDEnginePoloniex::wait_for_stop);
}
