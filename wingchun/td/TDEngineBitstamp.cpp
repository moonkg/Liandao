#include "TDEngineBitstamp.h"
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
#include <mutex>
#include <chrono>
#include <time.h>
#include <math.h>
#include <zlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <algorithm>
#include "../../utils/crypto/openssl_util.h"

using cpr::Post;
using cpr::Delete;
using cpr::Url;
using cpr::Body;
using cpr::Header;
using cpr::Parameters;
using cpr::Payload;
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
using utils::crypto::base64_url_encode;
USING_WC_NAMESPACE

typedef char char_64[64];

TDEngineBitstamp::TDEngineBitstamp(): ITDEngine(SOURCE_BITSTAMP)
{
    logger = yijinjing::KfLog::getLogger("TradeEngine.Bitstamp");
    KF_LOG_INFO(logger, "[TDEngineBitstamp]");

    mutex_order_and_trade = new std::mutex();
    mutex_response_order_status = new std::mutex();
    mutex_orderaction_waiting_response = new std::mutex();
}

TDEngineBitstamp::~TDEngineBitstamp()
{
    if(mutex_order_and_trade != nullptr) delete mutex_order_and_trade;
    if(mutex_response_order_status != nullptr) delete mutex_response_order_status;
    if(mutex_orderaction_waiting_response != nullptr) delete mutex_orderaction_waiting_response;
}
/*
static TDEngineBitstamp* global_md = nullptr;
//web socket代码
static int ws_service_cb( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
    std::stringstream ss;
    ss << "[ws_service_cb] lws_callback,reason=" << reason << ",";
    switch( reason )
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
        {//lws callback client established
            global_md->on_lws_open(wsi);
            //lws_callback_on_writable( wsi );
            break;
        }
        case LWS_CALLBACK_PROTOCOL_INIT:
        {//lws callback protocol init
            ss << "LWS_CALLBACK_PROTOCOL_INIT.";
            //global_md->writeInfoLog(ss.str());
            break;
        }
        case LWS_CALLBACK_CLIENT_RECEIVE:
        {//lws callback client receive
            ss << "LWS_CALLBACK_CLIENT_RECEIVE.";
            //global_md->writeInfoLog(ss.str());
            if(global_md)
            {//统一接收，不同订阅返回数据不同解析
                global_md->on_lws_data(wsi, (char *)in, len);
            }
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE:
        {//lws callback client writeable
            ss << "LWS_CALLBACK_CLIENT_WRITEABLE.";
            //global_md->writeInfoLog(ss.str());
            int ret = 0;
            if(global_md)
            {//统一发送，不同订阅不同请求
                ret = global_md->on_lws_write_subscribe(wsi);
            }
            break;
        }
        case LWS_CALLBACK_CLOSED:
        {//lws callback close
            ss << "LWS_CALLBACK_CLOSED.";
            global_md->on_lws_close(wsi);
            break;
        }
        case LWS_CALLBACK_WSI_DESTROY:
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        {//lws callback client connection error
            ss << "LWS_CALLBACK_CLIENT_CONNECTION_ERROR.";
            //global_md->writeInfoLog(ss.str());
            if(global_md)
            {
                global_md->on_lws_connection_error(wsi);
            }
            break;
        }
        default:
            ss << "default Info.";
            //global_md->writeInfoLog(ss.str());
            break;
    }

    return 0;
}
void TDEngineKraken::on_lws_open(struct lws* wsi){
    KF_LOG_INFO(logger,"[on_lws_open] ");
    //krakenAuth(findAccountUnitKrakenByWebsocketConn(wsi));
    KF_LOG_INFO(logger,"[on_lws_open] finished ");
}
//cys websocket connect
void TDEngineKraken::Ping(struct lws* conn)
{
    StringBuffer sbPing;
    Writer<StringBuffer> writer(sbPing);
    writer.StartObject();
    writer.Key("type");
    writer.String("ping");
    writer.EndObject();
    std::string strPing = sbPing.GetString();
    unsigned char msg[512];
    memset(&msg[LWS_PRE], 0, 512-LWS_PRE);
    int length = strPing.length();
    KF_LOG_INFO(logger, "TDEngineKraken::lws_write_ping: " << strPing.c_str() << " ,len = " << length);
    strncpy((char *)msg+LWS_PRE, strPing.c_str(), length);
    int ret = lws_write(conn, &msg[LWS_PRE], length,LWS_WRITE_TEXT);
}
void TDEngineKraken::Pong(struct lws* conn,long long ping){
    KF_LOG_INFO(logger,"[Pong] pong the ping of websocket");
    StringBuffer sbPing;
    Writer<StringBuffer> writer(sbPing);
    writer.StartObject();
    writer.Key("op");
    writer.String("pong");
    writer.Key("ts");
    writer.Int64(ping);
    writer.EndObject();
    std::string strPong = sbPing.GetString();
    unsigned char msg[512];
    memset(&msg[LWS_PRE], 0, 512-LWS_PRE);
    int length = strPong.length();
    KF_LOG_INFO(logger, "[Pong] data " << strPong.c_str() << " ,len = " << length);
    strncpy((char *)msg+LWS_PRE, strPong.c_str(), length);
    int ret = lws_write(conn, &msg[LWS_PRE], length,LWS_WRITE_TEXT);
}

void TDEngineKraken::on_lws_data(struct lws* conn, const char* data, size_t len)
{
    KF_LOG_INFO(logger, "[on_lws_data] (data) " << data);
    //std::string strData = dealDataSprit(data);
    Document json;
    json.Parse(data);
    if(json.HasParseError()||!json.IsObject()){
        KF_LOG_ERROR(logger, "[cys_on_lws_data] parse to json error ");
        return;
    }

}
std::string TDEngineKraken::makeSubscribeOrdersUpdate(AccountUnitKraken& unit){
    StringBuffer sbUpdate;
    Writer<StringBuffer> writer(sbUpdate);
    writer.StartObject();
    writer.Key("op");
    writer.String("sub");
    writer.Key("cid");
    writer.String(unit.spotAccountId.c_str());
    writer.Key("topic");
    writer.String("orders.*");
    writer.Key("model");
    writer.String("0");
    writer.EndObject();
    std::string strUpdate = sbUpdate.GetString();
    return strUpdate;
}
AccountUnitKraken& TDEngineKraken::findAccountUnitKrakenByWebsocketConn(struct lws * websocketConn){
    for (size_t idx = 0; idx < account_units.size(); idx++) {
        AccountUnitKraken &unit = account_units[idx];
        if(unit.webSocketConn == websocketConn) {
            return unit;
        }
    }
    return account_units[0];
}
int TDEngineKraken::subscribeTopic(struct lws* conn,string strSubscribe){
    unsigned char msg[1024];
    memset(&msg[LWS_PRE], 0, 1024-LWS_PRE);
    int length = strSubscribe.length();
    KF_LOG_INFO(logger, "[on_lws_write_subscribe] " << strSubscribe.c_str() << " ,len = " << length);
    strncpy((char *)msg+LWS_PRE, strSubscribe.c_str(), length);
    //请求
    int ret = lws_write(conn, &msg[LWS_PRE], length,LWS_WRITE_TEXT);
    lws_callback_on_writable(conn);
    return ret;
}
int TDEngineKraken::on_lws_write_subscribe(struct lws* conn){
    //KF_LOG_INFO(logger, "[on_lws_write_subscribe]" );
    int ret = 0;
    AccountUnitKraken& unit=findAccountUnitKrakenByWebsocketConn(conn);
    if(isAuth==kraken_auth&&isOrders != orders_sub){
        isOrders=orders_sub;
        string strSubscribe = makeSubscribeOrdersUpdate(unit);
        ret=subscribeTopic(conn,strSubscribe);
    }
    return ret;
}

void TDEngineKraken::on_lws_connection_error(struct lws* conn){
    KF_LOG_ERROR(logger, "TDEngineKraken::on_lws_connection_error. login again.");
    //clear the price book, the new websocket will give 200 depth on the first connect, it will make a new price book
    m_isPong = false;
    isAuth = nothing;isOrders=nothing;
    long timeout_nsec = 0;
    AccountUnitKraken& unit=findAccountUnitKrakenByWebsocketConn(conn);
    lws_login(unit,0);
}
void TDEngineKraken::on_lws_close(struct lws* conn){
    isAuth=nothing;isOrders=nothing;
    KF_LOG_INFO(logger,"[websocket close]");
}
static struct lws_protocols protocols[] =
        {
                {
                        "ws",
                        ws_service_cb,
                              0,
                                 65536,
                },
                { NULL, NULL, 0, 0 } 
        };
int on_lws_write_subscribe(struct lws* conn);
void on_lws_connection_error(struct lws* conn);

struct session_data {
    int fd;
};*/
void TDEngineBitstamp::writeInfoLog(std::string strInfo){
    KF_LOG_INFO(logger,strInfo);
}
void TDEngineBitstamp::writeErrorLog(std::string strError)
{
    KF_LOG_ERROR(logger, strError);
}

int64_t TDEngineBitstamp::getMSTime(){
    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return  timestamp;
}
// helper function to compute SHA256:
std::vector<unsigned char> TDEngineBitstamp::sha256(string& data){
   std::vector<unsigned char> digest(SHA256_DIGEST_LENGTH);
   SHA256_CTX ctx;
   SHA256_Init(&ctx);
   SHA256_Update(&ctx, data.c_str(), data.length());
   SHA256_Final(digest.data(), &ctx);

   return digest;
}
/*
vector<unsigned char> TDEngineBitstamp::hmac_sha512_bitstamp(vector<unsigned char>& data,vector<unsigned char> key){   
   unsigned int len = EVP_MAX_MD_SIZE;
   std::vector<unsigned char> digest(len);

   HMAC_CTX ctx;
   HMAC_CTX_init(&ctx);

   HMAC_Init_ex(&ctx, key.data(), key.size(), EVP_sha512(), NULL);
   HMAC_Update(&ctx, data.data(), data.size());
   HMAC_Final(&ctx, digest.data(), &len);
   
   HMAC_CTX_cleanup(&ctx);
   
   return digest;
}
std::string TDEngineBitstamp::b64_encode(const std::vector<unsigned char>& data) {
   BIO* b64 = BIO_new(BIO_f_base64());
   BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

   BIO* bmem = BIO_new(BIO_s_mem());
   b64 = BIO_push(b64, bmem);
   
   BIO_write(b64, data.data(), data.size());
   BIO_flush(b64);

   BUF_MEM* bptr = NULL;
   BIO_get_mem_ptr(b64, &bptr);
   
   std::string output(bptr->data, bptr->length);
   BIO_free_all(b64);

   return output;
}
std::vector<unsigned char> TDEngineBitstamp::b64_decode(const std::string& data) {
   BIO* b64 = BIO_new(BIO_f_base64());
   BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

   BIO* bmem = BIO_new_mem_buf((void*)data.c_str(),data.length());
   bmem = BIO_push(b64, bmem);
   
   std::vector<unsigned char> output(data.length());
   int decoded_size = BIO_read(bmem, output.data(), output.size());
   BIO_free_all(bmem);

   if (decoded_size < 0)
      throw std::runtime_error("failed while decoding base64.");
   
   return output;
}
std::string TDEngineBitstamp::getBitstampSignature(std::string& path,std::string& nonce, std::string postdata,AccountUnitBitstamp& unit){
   // add path to data to encrypt
   std::vector<unsigned char> data(path.begin(), path.end());

   // concatenate nonce and postdata and compute SHA256
   string np=nonce + postdata;
   std::vector<unsigned char> nonce_postdata = sha256(np);

   // concatenate path and nonce_postdata (path + sha256(nonce + postdata))
   data.insert(data.end(), nonce_postdata.begin(), nonce_postdata.end());

   // and compute HMAC
   return b64_encode(hmac_sha512_bitstamp(data, b64_decode(unit.secret_key)));
}*/
//cys edit from kraken api
std::mutex g_httpMutex;
cpr::Response TDEngineBitstamp::Get(const std::string& method_url,const std::string& body, std::string postData,AccountUnitBitstamp& unit)
{
    string url = unit.baseUrl + method_url+"?"+postData;
    std::unique_lock<std::mutex> lock(g_httpMutex);
    const auto response = cpr::Get(Url{url},
                                   Header{{}}, Timeout{10000} );
    lock.unlock();
    //if(response.text.length()<500){
    KF_LOG_INFO(logger, "[Get] (url) " << url << " (response.status_code) " << response.status_code <<
        " (response.error.message) " << response.error.message <<" (response.text) " << response.text.c_str());
    //}
    return response;
}
//cys edit
cpr::Response TDEngineBitstamp::Post(const std::string& method_url,const std::string& body,std::string postData, AccountUnitBitstamp& unit)
{
    /*int64_t nonce = getTimestamp();
    string nonceStr=std::to_string(nonce);
    KF_LOG_INFO(logger,"[Post] (nonce) "<<nonceStr);
    string s1="nonce=";
    postData=s1+nonceStr+"&"+postData;
    string path = method_url;
    string strSignature=getBitstampSignature(path,nonceStr,postData,unit);
    KF_LOG_INFO(logger,"[Post] (strSignature) "<<strSignature);*/


    string url = method_url;
    std::unique_lock<std::mutex> lock(g_httpMutex);
    auto response = cpr::Post(Url{url}, 
                                /*Header{
                                {"API-Key", unit.api_key},
                                {"API-Sign",strSignature}},*/
                                Body{body},Timeout{30000});
    lock.unlock();
    //if(response.text.length()<500){
    KF_LOG_INFO(logger, "[POST] (url) " << url <<" (body) "<< body<< " \n(response.status_code) " << response.status_code
        <<" (response.error.message) " << response.error.message <<" (response.text) " << response.text.c_str());
    //}
    return response;
}
void TDEngineBitstamp::init()
{
    genUniqueKey();
    ITDEngine::init();
    JournalPair tdRawPair = getTdRawJournalPair(source_id);
    raw_writer = yijinjing::JournalSafeWriter::create(tdRawPair.first, tdRawPair.second, "RAW_" + name());
    KF_LOG_INFO(logger, "[init]");
}

void TDEngineBitstamp::pre_load(const json& j_config)
{
    KF_LOG_INFO(logger, "[pre_load]");
}

void TDEngineBitstamp::resize_accounts(int account_num)
{
    account_units.resize(account_num);
    KF_LOG_INFO(logger, "[resize_accounts]");
}

TradeAccount TDEngineBitstamp::load_account(int idx, const json& j_config)
{
    KF_LOG_INFO(logger, "[load_account]");
    // internal load
    string api_key = j_config["APIKey"].get<string>();
    string secret_key = j_config["SecretKey"].get<string>();
    //string passphrase = j_config["passphrase"].get<string>();
    //https://api.kraken.pro
    //string baseUrl = j_config["baseUrl"].get<string>();
    rest_get_interval_ms = j_config["rest_get_interval_ms"].get<int>();

    if(j_config.find("orderaction_max_waiting_seconds") != j_config.end()) {
        orderaction_max_waiting_seconds = j_config["orderaction_max_waiting_seconds"].get<int>();
    }
    KF_LOG_INFO(logger, "[load_account] (orderaction_max_waiting_seconds)" << orderaction_max_waiting_seconds);

    if(j_config.find("max_rest_retry_times") != j_config.end()) {
        max_rest_retry_times = j_config["max_rest_retry_times"].get<int>();
    }
    KF_LOG_INFO(logger, "[load_account] (max_rest_retry_times)" << max_rest_retry_times);


    if(j_config.find("retry_interval_milliseconds") != j_config.end()) {
        retry_interval_milliseconds = j_config["retry_interval_milliseconds"].get<int>();
    }
    KF_LOG_INFO(logger, "[load_account] (retry_interval_milliseconds)" << retry_interval_milliseconds);

    AccountUnitBitstamp& unit = account_units[idx];
    unit.api_key = api_key;
    unit.secret_key = secret_key;
    /*unit.passphrase = passphrase;
    unit.baseUrl = baseUrl;

    KF_LOG_INFO(logger, "[load_account] (api_key)" << api_key << " (baseUrl)" << unit.baseUrl 
                                                   << " (spotAccountId) "<<unit.spotAccountId
                                                   << " (marginAccountId) "<<unit.marginAccountId);*/


    unit.coinPairWhiteList.ReadWhiteLists(j_config, "whiteLists");
    unit.coinPairWhiteList.Debug_print();

    unit.positionWhiteList.ReadWhiteLists(j_config, "positionWhiteLists");
    unit.positionWhiteList.Debug_print();

    //display usage:
    if(unit.coinPairWhiteList.Size() == 0) {
        KF_LOG_ERROR(logger, "TDEngineBitstamp::load_account: please add whiteLists in kungfu.json like this :");
        KF_LOG_ERROR(logger, "\"whiteLists\":{");
        KF_LOG_ERROR(logger, "    \"strategy_coinpair(base_quote)\": \"exchange_coinpair\",");
        KF_LOG_ERROR(logger, "    \"btc_usdt\": \"btcusdt\",");
        KF_LOG_ERROR(logger, "     \"etc_eth\": \"etceth\"");
        KF_LOG_ERROR(logger, "},");
    }
    //test
    Document json;
    get_account(unit, json);
    //printResponse(json);
    //cancel_order(unit,"code","OCITZY-JMMFG-AT2MB3",json);
    //printResponse(json);
    //getPriceVolumePrecision(unit);
    // set up
    TradeAccount account = {};
    //partly copy this fields
    strncpy(account.UserID, api_key.c_str(), 16);
    strncpy(account.Password, secret_key.c_str(), 21);
    //web socket登陆
    //login(0);
    return account;
}

void TDEngineBitstamp::connect(long timeout_nsec)
{
    KF_LOG_INFO(logger, "[connect]");
    for (size_t idx = 0; idx < account_units.size(); idx++)
    {
        AccountUnitBitstamp& unit = account_units[idx];
        //unit.logged_in = true;
        KF_LOG_INFO(logger, "[connect] (api_key)" << unit.api_key);
        Document doc;
        cancel_all_orders(unit, doc);
        //get_account(unit,doc);
        if (!unit.logged_in)
        {
            //KF_LOG_INFO(logger, "[connect] (account id) "<<unit.spotAccountId<<" login.");
            //lws_login(unit, 0);
            //set true to for let the kungfuctl think td is running.
            unit.logged_in = true;
        }
    }
}

void TDEngineBitstamp::getPriceVolumePrecision(AccountUnitBitstamp& unit){
    KF_LOG_INFO(logger,"[getPriceVolumePrecision]");
    Document json;
    const auto response = Get("/0/public/AssetPairs","","",unit);
    json.Parse(response.text.c_str());
    int errLen=json["error"].Size();
    if(json.HasMember("result") && errLen == 0 &&json["result"].IsObject()){
        rapidjson::Value result=json["result"].GetObject();
        for (rapidjson::Value::ConstMemberIterator itr = result.MemberBegin();itr != result.MemberEnd(); ++itr){
            auto key = (itr->name).GetString();
            rapidjson::Value account=result[key].GetObject();
            PriceVolumePrecision stPriceVolumePrecision;
            stPriceVolumePrecision.symbol=account["altname"].GetString();
            std::string ticker = unit.coinPairWhiteList.GetKeyByValue(stPriceVolumePrecision.symbol);
            if(ticker.length()==0){
                //KF_LOG_ERROR(logger,"[getPriceVolumePrecision] (No such symbol in whitelist) "<<stPriceVolumePrecision.symbol);
                continue;
            }
            stPriceVolumePrecision.baseCurrency=account["base"].GetString();
            stPriceVolumePrecision.quoteCurrency=account["quote"].GetString();
            stPriceVolumePrecision.pricePrecision=account["pair_decimals"].GetInt();
            stPriceVolumePrecision.amountPrecision=account["lot_decimals"].GetInt();
            unit.mapPriceVolumePrecision.insert(std::make_pair(stPriceVolumePrecision.symbol,stPriceVolumePrecision));
            KF_LOG_INFO(logger,"[getPriceVolumePrecision] symbol "<<stPriceVolumePrecision.symbol);
        }
        KF_LOG_INFO(logger,"[getPriceVolumePrecision] (map size) "<<unit.mapPriceVolumePrecision.size());
    }
}
/*
void TDEngineBitstamp::lws_login(AccountUnitBitstamp& unit, long timeout_nsec){
    KF_LOG_INFO(logger, "[TDEngineKraken::lws_login]");
    global_md = this;
    isAuth=nothing;
    isOrders=nothing;
    global_md = this;
    int inputPort = 443;
    int logs = LLL_ERR | LLL_DEBUG | LLL_WARN;

    struct lws_context_creation_info ctxCreationInfo;
    struct lws_client_connect_info clientConnectInfo;
    //struct lws *wsi = NULL;

    memset(&ctxCreationInfo, 0, sizeof(ctxCreationInfo));
    memset(&clientConnectInfo, 0, sizeof(clientConnectInfo));

    ctxCreationInfo.port = CONTEXT_PORT_NO_LISTEN;
    ctxCreationInfo.iface = NULL;
    ctxCreationInfo.protocols = protocols;
    ctxCreationInfo.ssl_cert_filepath = NULL;
    ctxCreationInfo.ssl_private_key_filepath = NULL;
    ctxCreationInfo.extensions = NULL;
    ctxCreationInfo.gid = -1;
    ctxCreationInfo.uid = -1;
    ctxCreationInfo.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    ctxCreationInfo.fd_limit_per_thread = 1024;
    ctxCreationInfo.max_http_header_pool = 1024;
    ctxCreationInfo.ws_ping_pong_interval=1;
    ctxCreationInfo.ka_time = 10;
    ctxCreationInfo.ka_probes = 10;
    ctxCreationInfo.ka_interval = 10;

    context = lws_create_context(&ctxCreationInfo);
    KF_LOG_INFO(logger, "[TDEngineKraken::lws_login] context created.");


    if (context == NULL) {
        KF_LOG_ERROR(logger, "[TDEngineKraken::lws_login] context is NULL. return");
        return;
    }

    // Set up the client creation info
    static std::string host  = "api.kraken.pro";
    static std::string path = "/ws/v1";
    clientConnectInfo.address = host.c_str();
    clientConnectInfo.path = path.c_str(); // Set the info's path to the fixed up url path
    
    clientConnectInfo.context = context;
    clientConnectInfo.port = 443;
    clientConnectInfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
    clientConnectInfo.host = host.c_str();
    clientConnectInfo.origin = "origin";
    clientConnectInfo.ietf_version_or_minus_one = -1;
    clientConnectInfo.protocol = protocols[0].name;
    clientConnectInfo.pwsi = &unit.webSocketConn;

    KF_LOG_INFO(logger, "[TDEngineKraken::login] address = " << clientConnectInfo.address << ",path = " << clientConnectInfo.path);
    //建立websocket连接
    unit.webSocketConn = lws_client_connect_via_info(&clientConnectInfo);
    if (unit.webSocketConn == NULL) {
        KF_LOG_ERROR(logger, "[TDEngineKraken::lws_login] wsi create error.");
        return;
    }
    KF_LOG_INFO(logger, "[TDEngineKraken::login] wsi create success.");
}*/
void TDEngineBitstamp::login(long timeout_nsec)
{
    KF_LOG_INFO(logger, "[TDEngineBitstamp::login]");
    connect(timeout_nsec);
}

void TDEngineBitstamp::logout()
{
    KF_LOG_INFO(logger, "[logout]");
}

void TDEngineBitstamp::release_api()
{
    KF_LOG_INFO(logger, "[release_api]");
}

bool TDEngineBitstamp::is_logged_in() const{
    KF_LOG_INFO(logger, "[is_logged_in]");
    for (auto& unit: account_units)
    {
        if (!unit.logged_in)
            return false;
    }
    return true;
}

bool TDEngineBitstamp::is_connected() const{
    KF_LOG_INFO(logger, "[is_connected]");
    return is_logged_in();
}


std::string TDEngineBitstamp::GetSide(const LfDirectionType& input) {
    if (LF_CHAR_Buy == input) {
        return "buy";
    } else if (LF_CHAR_Sell == input) {
        return "sell";
    } else {
        return "";
    }
}

LfDirectionType TDEngineBitstamp::GetDirection(std::string input) {
    if ("buy" == input) {
        return LF_CHAR_Buy;
    } else if ("sell" == input) {
        return LF_CHAR_Sell;
    } else {
        return LF_CHAR_Buy;
    }
}

std::string TDEngineBitstamp::GetType(const LfOrderPriceTypeType& input) {
    if (LF_CHAR_LimitPrice == input) {
        return "limit";
    } else if (LF_CHAR_AnyPrice == input) {
        return "market";
    } else {
        return "";
    }
}

LfOrderPriceTypeType TDEngineBitstamp::GetPriceType(std::string input) {
    if ("limit" == input) {
        return LF_CHAR_LimitPrice;
    } else if ("market" == input) {
        return LF_CHAR_AnyPrice;
    } else {
        return '0';
    }
}
//订单状态，pending 提交, open 部分成交, closed , open 成交, canceled 已撤销,expired 失效
LfOrderStatusType TDEngineBitstamp::GetOrderStatus(std::string state) {

    if(state == "canceled"){
        return LF_CHAR_Canceled;
    }else if(state == "pending"){
        return LF_CHAR_NotTouched;
    }else if(state == "closed"){
        return LF_CHAR_Error;
    }else if(state == "expired"){
        return LF_CHAR_Error;
    }else if(state == "open"){
        return LF_CHAR_NoTradeQueueing;
    }
    return LF_CHAR_Unknown;
}

/**
 * req functions
 * 查询账户持仓
 *//*
void TDEngineBitstamp::req_investor_position(const LFQryPositionField* data, int account_index, int requestId){
    KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId);

    AccountUnitBitstamp& unit = account_units[account_index];
    unit.userref=std::to_string(requestId);
    KF_LOG_INFO(logger, "[req_investor_position] (api_key)" << unit.api_key << " (InstrumentID) " << data->InstrumentID);

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

    int errorId = 0;
    std::string errorMsg = "";
    Document d;
    get_account(unit, d);
    KF_LOG_INFO(logger, "[req_investor_position] (get_account)" );
    
    if(d.IsObject() && d.HasMember("error"))
    {
        size_t isError=d["error"].Size();
        errorId = 0;
        KF_LOG_INFO(logger, "[req_investor_position] (errorId) " << errorId);
        if(isError != 0) {
            errorId=520;
            if (d.HasMember("error") && d["error"].IsArray()) {
                int i;
                for(i=0;i<d["error"].Size();i++){
                    errorMsg=errorMsg+d["error"].GetArray()[i].GetString()+"\n";
                }
            }
            KF_LOG_ERROR(logger, "[req_investor_position] failed!" << " (rid)" << requestId << " (errorId)" << errorId
                                                                   << " (errorMsg) " << errorMsg);
            raw_writer->write_error_frame(&pos, sizeof(LFRspPositionField), source_id, MSG_TYPE_LF_RSP_POS_BITSTAMP, 1, requestId, errorId, errorMsg.c_str());
        }
    }
    send_writer->write_frame(data, sizeof(LFQryPositionField), source_id, MSG_TYPE_LF_QRY_POS_BITSTAMP, 1, requestId);
    //{"error":[],"result":{"ZEUR":"10.0000"}}
    std::vector<LFRspPositionField> tmp_vector;
    KF_LOG_INFO(logger, "[req_investor_position] (result)");
    if(!d.HasParseError() && d.HasMember("result")&&d["result"].IsObject())
    {
        Value accounts = d["result"].GetObject();
        for (rapidjson::Value::ConstMemberIterator itr = accounts.MemberBegin();itr != accounts.MemberEnd(); ++itr){
            //itr->name.GetString(), itr->value.GetType()
            std::string symbol = itr->name.GetString();
            KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol);
            pos.Position = std::round(std::stod(itr->value.GetString()) * scale_offset);
            tmp_vector.push_back(pos);
            KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId 
                << " (symbol) " << symbol << " (position) " << pos.Position);
        }
        
    }

    //send the filtered position
    int position_count = tmp_vector.size();
    if(position_count > 0) {
        for (int i = 0; i < position_count; i++) {
            on_rsp_position(&tmp_vector[i], i == (position_count - 1), requestId, errorId, errorMsg.c_str());
        }
    }
    else
    {
        KF_LOG_INFO(logger, "[req_investor_position] (!findSymbolInResult) (requestId)" << requestId);
        on_rsp_position(&pos, 1, requestId, errorId, errorMsg.c_str());
    }
}*/
void TDEngineBitstamp::req_investor_position(const LFQryPositionField* data, int account_index, int requestId)
{
    KF_LOG_INFO(logger, "[req_investor_position]");

    AccountUnitBitstamp& unit = account_units[account_index];
    KF_LOG_INFO(logger, "[req_investor_position] (api_key)" << unit.api_key);

    // User Balance
    Document d;
    get_account(unit, d);
    KF_LOG_INFO(logger, "[req_investor_position] get_account");
    printResponse(d);

    int errorId = 0;
    std::string errorMsg = "";
    if(d.HasParseError() )
    {
        errorId=100;
        errorMsg= "get_account http response has parse error. please check the log";
        KF_LOG_ERROR(logger, "[req_investor_position] get_account error! (rid)  -1 (errorId)" << errorId << " (errorMsg) " << errorMsg);
    }

    if(!d.HasParseError() && d.IsObject() && d.HasMember("code") && d["code"].IsNumber())
    {
        errorId = d["code"].GetInt();
        if(d.HasMember("msg") && d["msg"].IsString())
        {
            errorMsg = d["msg"].GetString();
        }

        KF_LOG_ERROR(logger, "[req_investor_position] get_account failed! (rid)  -1 (errorId)" << errorId << " (errorMsg) " << errorMsg);
    }
    send_writer->write_frame(data, sizeof(LFQryPositionField), source_id, MSG_TYPE_LF_QRY_POS_BITSTAMP, 1, requestId);

    LFRspPositionField pos;
    memset(&pos, 0, sizeof(LFRspPositionField));
    strncpy(pos.BrokerID, data->BrokerID, 11);
    strncpy(pos.InvestorID, data->InvestorID, 19);
    strncpy(pos.InstrumentID, data->InstrumentID, 31);
    pos.PosiDirection = LF_CHAR_Long;
    pos.Position = 0;

    std::vector<LFRspPositionField> tmp_vector;

    if(!d.HasParseError() && d.IsObject() && d.HasMember("balances"))
    {
        int len = d["balances"].Size();
        for ( int i  = 0 ; i < len ; i++ ) {
            std::string symbol = d["balances"].GetArray()[i]["asset"].GetString();
            std::string ticker = unit.positionWhiteList.GetKeyByValue(symbol);
            if(ticker.length() > 0) {
                strncpy(pos.InstrumentID, ticker.c_str(), 31);
                pos.Position = std::round(stod(d["balances"].GetArray()[i]["free"].GetString()) * scale_offset);
                tmp_vector.push_back(pos);
                KF_LOG_INFO(logger,  "[connect] (symbol)" << symbol << " (free)" <<  d["balances"].GetArray()[i]["free"].GetString()
                                                          << " (locked)" << d["balances"].GetArray()[i]["locked"].GetString());
                KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol << " (position) " << pos.Position);
            }
        }
    }
    if(!d.HasParseError())
    {
        if ( d.HasMember("bch_reserved") ) {
            std::string symbol = "bch";
            if(symbol.length() > 0) {
                strncpy(pos.InstrumentID, symbol.c_str(), 31);
                pos.Position = std::round(stod(d["bch_reserved"].GetString()) * scale_offset);
                tmp_vector.push_back(pos);
                KF_LOG_INFO(logger,  "[connect] (symbol)" << symbol);
                KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol << " (position) " << pos.Position);
            }
        }
        else if ( d.HasMember("btc_reserved") ) {
            std::string symbol = "btc";
            if(symbol.length() > 0) {
                strncpy(pos.InstrumentID, symbol.c_str(), 31);
                pos.Position = std::round(stod(d["btc_reserved"].GetString()) * scale_offset);
                tmp_vector.push_back(pos);
                KF_LOG_INFO(logger,  "[connect] (symbol)" << symbol);
                KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol << " (position) " << pos.Position);
            }
        }
        else if ( d.HasMember("eth_reserved") ) {
            std::string symbol = "eth";
            if(symbol.length() > 0) {
                strncpy(pos.InstrumentID, symbol.c_str(), 31);
                pos.Position = std::round(stod(d["eth_reserved"].GetString()) * scale_offset);
                tmp_vector.push_back(pos);
                KF_LOG_INFO(logger,  "[connect] (symbol)" << symbol);
                KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol << " (position) " << pos.Position);
            }
        }
        else if ( d.HasMember("eur_reserved") ) {
            std::string symbol = "eur";
            if(symbol.length() > 0) {
                strncpy(pos.InstrumentID, symbol.c_str(), 31);
                pos.Position = std::round(stod(d["eur_reserved"].GetString()) * scale_offset);
                tmp_vector.push_back(pos);
                KF_LOG_INFO(logger,  "[connect] (symbol)" << symbol);
                KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol << " (position) " << pos.Position);
            }
        }
        else if ( d.HasMember("ltc_reserved") ) {
            std::string symbol = "ltc";
            if(symbol.length() > 0) {
                strncpy(pos.InstrumentID, symbol.c_str(), 31);
                pos.Position = std::round(stod(d["ltc_reserved"].GetString()) * scale_offset);
                tmp_vector.push_back(pos);
                KF_LOG_INFO(logger,  "[connect] (symbol)" << symbol);
                KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol << " (position) " << pos.Position);
            }
        }
        else if ( d.HasMember("usd_reserved") ) {
            std::string symbol = "usd";
            if(symbol.length() > 0) {
                strncpy(pos.InstrumentID, symbol.c_str(), 31);
                pos.Position = std::round(stod(d["usd_reserved"].GetString()) * scale_offset);
                tmp_vector.push_back(pos);
                KF_LOG_INFO(logger,  "[connect] (symbol)" << symbol);
                KF_LOG_INFO(logger, "[req_investor_position] (requestId)" << requestId << " (symbol) " << symbol << " (position) " << pos.Position);
            }
        }
        else if ( d.HasMember("xrp_reserved") ) {
            std::string symbol = "xrp";
            if(symbol.length() > 0) {
                strncpy(pos.InstrumentID, symbol.c_str(), 31);
                pos.Position = std::round(stod(d["xrp_reserved"].GetString()) * scale_offset);
                tmp_vector.push_back(pos);
                KF_LOG_INFO(logger,  "[connect] (symbol)" << symbol);
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
        on_rsp_position(&pos, 1, requestId, errorId, errorMsg.c_str());
    }
    if(errorId != 0)
    {
        raw_writer->write_error_frame(&pos, sizeof(LFRspPositionField), source_id, MSG_TYPE_LF_RSP_POS_BITSTAMP, 1, requestId, errorId, errorMsg.c_str());
    }
}

void TDEngineBitstamp::req_qry_account(const LFQryAccountField *data, int account_index, int requestId)
{
    KF_LOG_INFO(logger, "[req_qry_account]");
}

void TDEngineBitstamp::dealPriceVolume(AccountUnitBitstamp& unit,const std::string& symbol,int64_t nPrice,int64_t nVolume,
            std::string& nDealPrice,std::string& nDealVolume){
    KF_LOG_DEBUG(logger, "[dealPriceVolume] (symbol)" << symbol);
    KF_LOG_DEBUG(logger, "[dealPriceVolume] (price)" << nPrice);
    KF_LOG_DEBUG(logger, "[dealPriceVolume] (volume)" << nVolume);
    std::string ticker = unit.coinPairWhiteList.GetValueByKey(symbol);
    auto it = unit.mapPriceVolumePrecision.find(ticker);
    if(it == unit.mapPriceVolumePrecision.end())
    {
        KF_LOG_INFO(logger, "[dealPriceVolume] symbol not find :" << ticker);
        nDealVolume = "0";
        return ;
    }
    else
    {
        KF_LOG_INFO(logger,"[dealPriceVolume] (deal price and volume precision)");
        int pPrecision=it->second.pricePrecision;
        int vPrecision=it->second.amountPrecision;
        KF_LOG_INFO(logger,"[dealPriceVolume] (pricePrecision) "<<pPrecision<<" (amountPrecision) "<<vPrecision);
        double tDealPrice=nPrice*1.0/scale_offset;
        double tDealVolume=nVolume*1.0/scale_offset;
        KF_LOG_INFO(logger,"[dealPriceVolume] (tDealPrice) "<<tDealPrice<<" (tDealVolume) "<<tDealVolume);
        char chP[16],chV[16];
        sprintf(chP,"%.8lf",nPrice*1.0/scale_offset);
        sprintf(chV,"%.8lf",nVolume*1.0/scale_offset);
        nDealPrice=chP;
        KF_LOG_INFO(logger,"[dealPriceVolume] (chP) "<<chP<<" (nDealPrice) "<<nDealPrice);
        nDealPrice=nDealPrice.substr(0,nDealPrice.find(".")+(pPrecision==0?pPrecision:(pPrecision+1)));
        nDealVolume=chV;
         KF_LOG_INFO(logger,"[dealPriceVolume]  (chP) "<<chV<<" (nDealVolume) "<<nDealVolume);
        nDealVolume=nDealVolume.substr(0,nDealVolume.find(".")+(vPrecision==0?vPrecision:(vPrecision+1)));
    }
    KF_LOG_INFO(logger, "[dealPriceVolume]  (symbol)" << ticker << " (Volume)" << nVolume << " (Price)" << nPrice
                                                      << " (FixedVolume)" << nDealVolume << " (FixedPrice)" << nDealPrice);
}
//发单

void TDEngineBitstamp::req_order_insert(const LFInputOrderField* data, int account_index, int requestId, long rcv_time){
    //on_rtn_order(NULL);
    AccountUnitBitstamp& unit = account_units[account_index];
    unit.userref=std::to_string(requestId);
    KF_LOG_DEBUG(logger, "[req_order_insert]" << " (rid)" << requestId
                                              << " (APIKey)" << unit.api_key
                                              << " (Tid)" << data->InstrumentID
                                              << " (Volume)" << data->Volume
                                              << " (LimitPrice)" << data->LimitPrice
                                              << " (OrderRef)" << data->OrderRef);
    send_writer->write_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER_BITSTAMP, 1, requestId);

    int errorId = 0;
    std::string errorMsg = "";

    std::string ticker = unit.coinPairWhiteList.GetValueByKey(std::string(data->InstrumentID));
    if(ticker.length() == 0) {
        errorId = 200;
        errorMsg = std::string(data->InstrumentID) + " not in WhiteList, ignore it";
        KF_LOG_ERROR(logger, "[req_order_insert]: not in WhiteList, ignore it  (rid)" << requestId <<
                                                                                      " (errorId)" << errorId << " (errorMsg) " << errorMsg);
        on_rsp_order_insert(data, requestId, errorId, errorMsg.c_str());
        raw_writer->write_error_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER_BITSTAMP, 1, requestId, errorId, errorMsg.c_str());
        return;
    }
    KF_LOG_DEBUG(logger, "[req_order_insert] (exchange_ticker)" << ticker);
    Document d;
    double fixedPrice = data->LimitPrice*1.0/scale_offset;
    double  fixedVolume = data->Volume*1.0/scale_offset;

    std::string priceStr;
    std::stringstream convertPriceStream;
    convertPriceStream <<std::fixed << std::setprecision(2) << fixedPrice;
    convertPriceStream >> priceStr;

    std::string sizeStr;
    std::stringstream convertSizeStream;
    convertSizeStream <<std::fixed << std::setprecision(2) << fixedVolume;
    convertSizeStream >> sizeStr;

    KF_LOG_INFO(logger,"[req_order_insert] cys_ticker "<<ticker.c_str()<<" "<<GetSide(data->Direction)<<" "<<GetType(data->OrderPriceType));
    //lock
    if((GetSide(data->Direction)=="buy")&&(GetType(data->OrderPriceType)=="limit")) {
        send_buylimitorder(unit, unit.userref, ticker, GetSide(data->Direction),GetType(data->OrderPriceType), sizeStr, priceStr, d);
    }
    else if((GetSide(data->Direction)=="buy")&&(GetType(data->OrderPriceType)=="market")) {
        send_buymarketorder(unit, unit.userref, ticker, GetSide(data->Direction),GetType(data->OrderPriceType), sizeStr, priceStr, d);
    }
    else if((GetSide(data->Direction)=="sell")&&(GetType(data->OrderPriceType)=="limit")) { 
        send_selllimitorder(unit, unit.userref, ticker, GetSide(data->Direction),GetType(data->OrderPriceType), sizeStr, priceStr, d);
    }
    else if((GetSide(data->Direction)=="sell")&&(GetType(data->OrderPriceType)=="market")) {
        send_sellmarketorder(unit, unit.userref, ticker, GetSide(data->Direction),GetType(data->OrderPriceType), sizeStr, priceStr, d);
    }
    /*std::map<std::string, std::string>::iterator itr = localOrderRefRemoteOrderId.find(data->OrderRef);
    std::string remoteOrderId;
    if(itr == localOrderRefRemoteOrderId.end()) {
        return;
    } else {
        remoteOrderId = itr->second;
        KF_LOG_DEBUG(logger, " [order_status](remoteOrderId) " << remoteOrderId);
    }*/

    //Document d;
    //order_status(unit, ticker, remoteOrderId, d);

    //not expected response
    string errorstr="error";
    if(!d.IsObject()){
        errorId = 100;
        errorMsg = "send_order http response has parse error or is not json. please check the log";
        KF_LOG_ERROR(logger, "[req_order_insert] send_order error!  (rid)" << requestId << " (errorId)" <<
                                                                           errorId << " (errorMsg) " << errorMsg);
    }
    if(d.HasMember("status")&&d["status"].GetString()==errorstr){
        KF_LOG_INFO(logger,"[req_order_insert] (error) ");
    }
    else {
            //if send successful and the exchange has received ok, then add to  pending query order list
            //std::string remoteOrderId = result["txid"].GetArray()[0].GetString();
            //fix defect of use the old value
            /*localOrderRefRemoteOrderId[std::string(data->OrderRef)] = remoteOrderId;
            KF_LOG_INFO(logger, "[req_order_insert] after send  (rid)" << requestId << " (OrderRef) " <<
                                                                       data->OrderRef << " (remoteOrderId) "
                                                                       << remoteOrderId);*/
            std::string remoteOrderId = d["id"].GetString();
            localOrderRefRemoteOrderId[std::string(data->OrderRef)] = remoteOrderId;
            PendingOrderStatus pOrderStatus;
            //初始化
            memset(&pOrderStatus, 0, sizeof(PendingOrderStatus));
            LFRtnOrderField *rtn_order = &pOrderStatus.rtn_order;
            strncpy(rtn_order->BusinessUnit,remoteOrderId.c_str(),21);
            rtn_order->OrderStatus = LF_CHAR_NotTouched;
            rtn_order->VolumeTraded = 0;
            rtn_order->VolumeTotalOriginal = data->Volume;
            rtn_order->LimitPrice = data->LimitPrice;
            
            strcpy(rtn_order->ExchangeID, "bitstamp");
            strncpy(rtn_order->UserID, unit.api_key.c_str(), 16);
            strncpy(rtn_order->InstrumentID, data->InstrumentID, 31);
            rtn_order->Direction = data->Direction;
            //No this setting on Bitstamp
            rtn_order->TimeCondition = LF_CHAR_GTC;
            rtn_order->OrderPriceType = data->OrderPriceType;
            strncpy(rtn_order->OrderRef, data->OrderRef, 13);
            rtn_order->VolumeTotalOriginal = data->Volume;
            rtn_order->LimitPrice = data->LimitPrice;
            rtn_order->VolumeTotal = data->Volume;

            on_rtn_order(rtn_order);
            raw_writer->write_frame(rtn_order, sizeof(LFRtnOrderField),
                                    source_id, MSG_TYPE_LF_RTN_TRADE_BITSTAMP,
                                    1, (rtn_order->RequestID > 0) ? rtn_order->RequestID : -1);

            KF_LOG_DEBUG(logger, "[req_order_insert] (addNewQueryOrdersAndTrades)" );
            pOrderStatus.averagePrice = 0;
            addNewQueryOrdersAndTrades(unit, pOrderStatus, remoteOrderId);

            raw_writer->write_error_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER_BITSTAMP, 1,
                                          requestId, errorId, errorMsg.c_str());
            KF_LOG_DEBUG(logger, "[req_order_insert] success" );
            //return;
        }
    //unlock
    if(errorId != 0)
    {
        on_rsp_order_insert(data, requestId, errorId, errorMsg.c_str());
        raw_writer->write_error_frame(data, sizeof(LFInputOrderField), source_id, MSG_TYPE_LF_ORDER_BITSTAMP, 1, requestId, errorId, errorMsg.c_str());
    }
}

void TDEngineBitstamp::req_order_action(const LFOrderActionField* data, int account_index, int requestId, long rcv_time){
    AccountUnitBitstamp& unit = account_units[account_index];
    unit.userref=std::to_string(requestId);
    KF_LOG_DEBUG(logger, "[req_order_action]" << " (rid)" << requestId
                                              << " (APIKey)" << unit.api_key
                                              << " (Iid)" << data->InvestorID
                                              << " (OrderRef)" << data->OrderRef
                                              << " (KfOrderID)" << data->KfOrderID);

    send_writer->write_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_BITSTAMP, 1, requestId);

    int errorId = 0;
    std::string errorMsg = "";

    std::string ticker = unit.coinPairWhiteList.GetValueByKey(std::string(data->InstrumentID));
    if(ticker.length() == 0) {
        errorId = 200;
        errorMsg = std::string(data->InstrumentID) + " not in WhiteList, ignore it";
        KF_LOG_ERROR(logger, "[req_order_action]: not in WhiteList , ignore it: (rid)" << requestId << " (errorId)" <<
                                                                                       errorId << " (errorMsg) " << errorMsg);
        on_rsp_order_action(data, requestId, errorId, errorMsg.c_str());
        raw_writer->write_error_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_BITSTAMP, 1, requestId, errorId, errorMsg.c_str());
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
        raw_writer->write_error_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_BITSTAMP, 1, requestId, errorId, errorMsg.c_str());
        return;
    } else {
        remoteOrderId = itr->second;
        KF_LOG_DEBUG(logger, "[req_order_action] found in localOrderRefRemoteOrderId map (orderRef) "
                << data->OrderRef << " (remoteOrderId) " << remoteOrderId);
    }
    /*std::vector<PendingOrderStatus>::iterator itr1;
    for(itr1 = unit.pendingOrderStatus.begin(); itr1 != unit.pendingOrderStatus.end();)
    {
        std::string ticker = unit.coinPairWhiteList.GetValueByKey(std::string(itr1->rtn_order.InstrumentID));
        if(ticker.length() == 0) {
            itr1 = unit.pendingOrderStatus.erase(itr1);
            continue;
        }
        
        string remoteOrderId = itr1->rtn_order.BusinessUnit;*/

    Document d;
    cancel_order(unit, ticker, remoteOrderId, d);

    if(!d.HasParseError()&&d.HasMember("id")) {
        errorId = 0;
        //rapidjson::Value result = d["result"].GetObject();
        //int count = result["count"].GetInt();
        std::vector<PendingOrderStatus>::iterator itr;
        for(itr = unit.pendingOrderStatus.begin(); itr != unit.pendingOrderStatus.end();){
            string oldRemoteOrderId=itr->rtn_order.BusinessUnit;
            if(remoteOrderId == oldRemoteOrderId){
                KF_LOG_INFO(logger,"[req_order_action] (orderIsCanceled)");
                orderIsCanceled(unit,&(itr->rtn_order));
                unit.pendingOrderStatus.erase(itr);
            }else{
                itr++;
            }
        }
        KF_LOG_INFO(logger,"[req_order_action] (canceled order counts) ");
    }else{
        //errorId = 520;
        if (d.HasMember("error")) {
            KF_LOG_ERROR(logger, "[req_order_action] cancel_order failed!");
            //errorId = 520;
        }
        /*KF_LOG_ERROR(logger, "[req_order_action] cancel_order failed!" << " (rid)" << requestId
                                                                       << " (errorId)" << errorId << " (errorMsg) " << errorMsg);*/
    }
    if(errorId != 0)
    {
        on_rsp_order_action(data, requestId, errorId, errorMsg.c_str());
        raw_writer->write_error_frame(data, sizeof(LFOrderActionField), source_id, MSG_TYPE_LF_ORDER_ACTION_BITSTAMP, 1, requestId, errorId, errorMsg.c_str());

    } else {
        //itr1++;
        KF_LOG_INFO(logger,"[req_order_action] cancel order success");
    }

}
//对于每个撤单指令发出后30秒（可配置）内，如果没有收到回报，就给策略报错（撤单被拒绝，pls retry)
void TDEngineBitstamp::addRemoteOrderIdOrderActionSentTime(const LFOrderActionField* data, int requestId, const std::string& remoteOrderId){
    std::lock_guard<std::mutex> guard_mutex_order_action(*mutex_orderaction_waiting_response);

    OrderActionSentTime newOrderActionSent;
    newOrderActionSent.requestId = requestId;
    newOrderActionSent.sentNameTime = getTimestamp();
    memcpy(&newOrderActionSent.data, data, sizeof(LFOrderActionField));
    remoteOrderIdOrderActionSentTime[remoteOrderId] = newOrderActionSent;
}
//cys no use
void TDEngineBitstamp::GetAndHandleOrderTradeResponse(){
    // KF_LOG_INFO(logger, "[GetAndHandleOrderTradeResponse]" );
    //every account
    for (size_t idx = 0; idx < account_units.size(); idx++)
    {
        AccountUnitBitstamp& unit = account_units[idx];
        if (!unit.logged_in)
        {
            continue;
        }
        //将新订单放到提交缓存中
        moveNewOrderStatusToPending(unit);
        retrieveOrderStatus(unit);
    }//end every account
}

//订单状态cys not use
void TDEngineBitstamp::retrieveOrderStatus(AccountUnitBitstamp& unit){

    std::lock_guard<std::mutex> guard_mutex(*mutex_response_order_status);
    std::lock_guard<std::mutex> guard_mutex_order_action(*mutex_orderaction_waiting_response);

    std::vector<PendingOrderStatus>::iterator orderStatusIterator;
    for(orderStatusIterator = unit.pendingOrderStatus.begin(); orderStatusIterator != unit.pendingOrderStatus.end();)
    {

        std::string ticker = unit.coinPairWhiteList.GetValueByKey(std::string(orderStatusIterator->rtn_order.InstrumentID));
        if(ticker.length() == 0) {
            KF_LOG_INFO(logger, "[retrieveOrderStatus]: not in WhiteList , ignore it:" << orderStatusIterator->rtn_order.InstrumentID);
            orderStatusIterator = unit.pendingOrderStatus.erase(orderStatusIterator);
            continue;
        }
        KF_LOG_INFO(logger, "[retrieveOrderStatus] get_order " << "( account.api_key) " << unit.api_key
                                                               << "  (account.pendingOrderStatus.InstrumentID) " << orderStatusIterator->rtn_order.InstrumentID
                                                               << "  (account.pendingOrderStatus.OrderRef) " << orderStatusIterator->rtn_order.OrderRef
                                                               << "  (account.pendingOrderStatus.remoteOrderId) " << orderStatusIterator->rtn_order.BusinessUnit
                                                               << "  (account.pendingOrderStatus.OrderStatus) " << orderStatusIterator->rtn_order.OrderStatus
                                                               << "  (exchange_ticker)" << ticker
        );
        string remoteOrderId = orderStatusIterator->rtn_order.BusinessUnit;
        Document d;
        query_order(unit, ticker,remoteOrderId, d);
        //订单状态，pending 提交, open 成交, canceled 已撤销, expired已失效, closed 
        if(d.HasParseError()) {
            //HasParseError, skip
            KF_LOG_ERROR(logger, "[retrieveOrderStatus] get_order response HasParseError " << " (symbol)" << orderStatusIterator->rtn_order.InstrumentID
                                                                                           << " (orderRef)" << orderStatusIterator->rtn_order.OrderRef
                                                                                           << " (remoteOrderId) " << remoteOrderId);
            continue;
        }
        KF_LOG_INFO(logger, "[retrieveOrderStatus] query_order:");
        string finishedstr = "Finished";
        string openstr = "Open";
        string queuestr = "In Queue";
        
        string tickerstr=ticker.substr(0,3);
        char_64 tickerchar;
        strcpy(tickerchar,tickerstr.c_str());
        if(d.HasMember("transactions")&&d["transactions"].IsArray()&&(d["transactions"].Size()!=0)&&(d["status"].GetString()==finishedstr))
        {
                Value &node=d["transactions"]; 

                KF_LOG_INFO(logger, "[retrieveOrderStatus] (query success)");
                ResponsedOrderStatus responsedOrderStatus;
                responsedOrderStatus.ticker = ticker;

                //平均价格
                responsedOrderStatus.averagePrice = std::round(std::stod(node.GetArray()[0]["price"].GetString()) * scale_offset);
                //累计成交价格
                responsedOrderStatus.PriceTraded = std::round(std::stod(node.GetArray()[0]["usd"].GetString()) * scale_offset);
                //总量
                responsedOrderStatus.volume = std::round(orderStatusIterator->rtn_order.VolumeTotalOriginal);
                //累计成交数量
                responsedOrderStatus.VolumeTraded = std::round(std::stod(node.GetArray()[0][tickerchar].GetString()) * scale_offset);
                //未成交数量
                responsedOrderStatus.openVolume =  responsedOrderStatus.volume - orderStatusIterator->rtn_order.VolumeTraded;
                //订单状态
                //responsedOrderStatus.OrderStatus = LF_CHAR_AllTraded;

                int len = d["transactions"].Size();
                if(len>1){
                    for(int i=1;i<len;i++){
                        //累计成交价格
                        responsedOrderStatus.PriceTraded += std::round(std::stod(node.GetArray()[i]["usd"].GetString()) * scale_offset);
                        //累计成交数量
                        responsedOrderStatus.VolumeTraded += std::round(std::stod(node.GetArray()[i][tickerchar].GetString()) * scale_offset);
                        //未成交数量
                        responsedOrderStatus.openVolume =  responsedOrderStatus.volume - orderStatusIterator->rtn_order.VolumeTraded;                                        
                    }
                }
                KF_LOG_ERROR(logger, "[volume and VolumeTraded]  " << " (Volume)" << responsedOrderStatus.volume
                                                                   << " (VolumeTraded)" << responsedOrderStatus.VolumeTraded);
                                                                                  
                //订单状态
                if(responsedOrderStatus.volume==responsedOrderStatus.VolumeTraded){
                    responsedOrderStatus.OrderStatus = LF_CHAR_AllTraded;
                }
                else if(responsedOrderStatus.volume>responsedOrderStatus.VolumeTraded){
                    responsedOrderStatus.OrderStatus = LF_CHAR_Canceled;
                }
                //订单信息处理
                handlerResponseOrderStatus(unit, orderStatusIterator, responsedOrderStatus);

                //OrderAction发出以后，有状态回来，就清空这次OrderAction的发送状态，不必制造超时提醒信息
                remoteOrderIdOrderActionSentTime.erase(orderStatusIterator->rtn_order.BusinessUnit);
            
        } 
        else if(d.HasMember("transactions")&&d["transactions"].IsArray()&&(d["transactions"].Size()==0))
        {
                KF_LOG_INFO(logger, "[retrieveOrderStatus] (query success)");
                Value &node=d["transactions"]; 
                ResponsedOrderStatus responsedOrderStatus;
                responsedOrderStatus.ticker = ticker;
                //平均价格
                responsedOrderStatus.averagePrice = 0;
                //累计成交价格
                responsedOrderStatus.PriceTraded = 0;
                //总量
                responsedOrderStatus.volume = std::round(orderStatusIterator->rtn_order.VolumeTotalOriginal);
                //累计成交数量
                responsedOrderStatus.VolumeTraded = 0;
                //未成交数量
                responsedOrderStatus.openVolume =  responsedOrderStatus.volume - orderStatusIterator->rtn_order.VolumeTraded;
                //订单状态
                responsedOrderStatus.OrderStatus = LF_CHAR_NotTouched;
                //订单信息处理
                handlerResponseOrderStatus(unit, orderStatusIterator, responsedOrderStatus);

                //OrderAction发出以后，有状态回来，就清空这次OrderAction的发送状态，不必制造超时提醒信息
                remoteOrderIdOrderActionSentTime.erase(orderStatusIterator->rtn_order.BusinessUnit);
            
        }
        else if(d.HasMember("transactions")&&d["transactions"].IsArray()&&(d["transactions"].Size()!=0)&&(d["status"].GetString()==openstr))
        {
                Value &node=d["transactions"]; 

                KF_LOG_INFO(logger, "[retrieveOrderStatus] (query success)");
                ResponsedOrderStatus responsedOrderStatus;
                responsedOrderStatus.ticker = ticker;
                //平均价格
                responsedOrderStatus.averagePrice = std::round(std::stod(node.GetArray()[0]["price"].GetString()) * scale_offset);
                //累计成交价格
                responsedOrderStatus.PriceTraded = std::round(std::stod(node.GetArray()[0]["usd"].GetString()) * scale_offset);
                //总量
                responsedOrderStatus.volume = std::round(orderStatusIterator->rtn_order.VolumeTotalOriginal);
                //累计成交数量
                responsedOrderStatus.VolumeTraded = std::round(std::stod(node.GetArray()[0][tickerchar].GetString()) * scale_offset);
                //未成交数量
                responsedOrderStatus.openVolume =  responsedOrderStatus.volume - orderStatusIterator->rtn_order.VolumeTraded;
                //订单状态
                responsedOrderStatus.OrderStatus = LF_CHAR_PartTradedQueueing;

                int len = d["transactions"].Size();
                if(len>1){
                    for(int i=1;i<len;i++){
                        //累计成交价格
                        responsedOrderStatus.PriceTraded += std::round(std::stod(node.GetArray()[i]["usd"].GetString()) * scale_offset);
                        //累计成交数量
                        responsedOrderStatus.VolumeTraded += std::round(std::stod(node.GetArray()[i][tickerchar].GetString()) * scale_offset);
                        //未成交数量
                        responsedOrderStatus.openVolume =  responsedOrderStatus.volume - orderStatusIterator->rtn_order.VolumeTraded;                                        
                    }
                }
                //订单信息处理
                handlerResponseOrderStatus(unit, orderStatusIterator, responsedOrderStatus);

                //OrderAction发出以后，有状态回来，就清空这次OrderAction的发送状态，不必制造超时提醒信息
                remoteOrderIdOrderActionSentTime.erase(orderStatusIterator->rtn_order.BusinessUnit);
            
        } 
        else if(d.HasMember("error")){
            KF_LOG_INFO(logger, "[retrieveOrderStatus] (query error)");
            orderIsCanceled(unit,&(orderStatusIterator->rtn_order));
        }
        /*else if(d["transactions"].IsArray()&&(d["transactions"].Size()==0)&&(d["status"].GetString()==openstr)){
            orderIsCanceled(unit,&(orderStatusIterator->rtn_order));
        }
         else if(d["transactions"].IsArray()&&(d["transactions"].Size()==0)&&(d["status"].GetString()==openstr)){
                Value &node=d["transactions"]; 

                KF_LOG_INFO(logger, "[retrieveOrderStatus] (query success)");
                ResponsedOrderStatus responsedOrderStatus;
                responsedOrderStatus.ticker = ticker;
                //平均价格
                //responsedOrderStatus.averagePrice = std::round(std::stod(node.GetArray()[0]["price"].GetString()) * scale_offset);
                //累计成交价格
                //responsedOrderStatus.PriceTraded = std::round(std::stod(node.GetArray()[0]["usd"].GetString()) * scale_offset);
                //总量
                //responsedOrderStatus.volume = std::round(std::stod(node.GetArray()[0]["xrp"].GetString()) * scale_offset);
                //累计成交数量
                //responsedOrderStatus.VolumeTraded = std::round(std::stod(node.GetArray()[0]["xrp"].GetString()) * scale_offset);
                //未成交数量
                //responsedOrderStatus.openVolume =  responsedOrderStatus.volume - orderStatusIterator->rtn_order.VolumeTraded;
                //订单状态
                responsedOrderStatus.OrderStatus = LF_CHAR_Canceled;
                //订单信息处理
                handlerResponseOrderStatus(unit, orderStatusIterator, responsedOrderStatus);

                //OrderAction发出以后，有状态回来，就清空这次OrderAction的发送状态，不必制造超时提醒信息
                remoteOrderIdOrderActionSentTime.erase(orderStatusIterator->rtn_order.BusinessUnit);            
        }*/
        /*else{
            KF_LOG_INFO(logger, "[retrieveOrderStatus] (query failed)");
            std::string errorMsg;
            std::string errorId = "520";
            if (d.HasMember("error") && d["error"].IsArray()) {
                int i;
                for(i=0;i<d["error"].Size();i++){
                    errorMsg=errorMsg+d["error"].GetArray()[i].GetString()+"\t";
                }
            }
            KF_LOG_ERROR(logger, "[retrieveOrderStatus] get_order fail." << " (symbol)" << orderStatusIterator->rtn_order.InstrumentID
                                                                         << " (orderRef)" << orderStatusIterator->rtn_order.OrderRef
                                                                         << " (errorId)" << errorId
                                                                         << " (errorMsg)" << errorMsg);
        }*/

        //remove order when finish
        if(orderStatusIterator->rtn_order.OrderStatus == LF_CHAR_AllTraded  || orderStatusIterator->rtn_order.OrderStatus == LF_CHAR_Canceled
           || orderStatusIterator->rtn_order.OrderStatus == LF_CHAR_Error)
        {
            KF_LOG_INFO(logger, "[retrieveOrderStatus] remove a pendingOrderStatus.");
            orderStatusIterator = unit.pendingOrderStatus.erase(orderStatusIterator);
        } else {
            ++orderStatusIterator;
        }
    }
}
void TDEngineBitstamp::addNewQueryOrdersAndTrades(AccountUnitBitstamp& unit, PendingOrderStatus pOrderStatus, std::string& remoteOrderId){
    KF_LOG_DEBUG(logger, "[addNewQueryOrdersAndTrades]" );
    //add new orderId for GetAndHandleOrderTradeResponse
    std::lock_guard<std::mutex> guard_mutex(*mutex_order_and_trade);

    unit.newOrderStatus.push_back(pOrderStatus);

    KF_LOG_INFO(logger, "[addNewQueryOrdersAndTrades] (InstrumentID) " << pOrderStatus.rtn_order.InstrumentID
                                                                       << " (OrderRef) " << pOrderStatus.rtn_order.OrderRef
                                                                       << " (remoteOrderId) " << pOrderStatus.rtn_order.BusinessUnit
                                                                       << "(VolumeTraded)" << pOrderStatus.rtn_order.VolumeTraded);
}


void TDEngineBitstamp::moveNewOrderStatusToPending(AccountUnitBitstamp& unit)
{
    std::lock_guard<std::mutex> pending_guard_mutex(*mutex_order_and_trade);
    std::lock_guard<std::mutex> response_guard_mutex(*mutex_response_order_status);


    std::vector<PendingOrderStatus>::iterator newOrderStatusIterator;
    for(newOrderStatusIterator = unit.newOrderStatus.begin(); newOrderStatusIterator != unit.newOrderStatus.end();)
    {
        unit.pendingOrderStatus.push_back(*newOrderStatusIterator);
        newOrderStatusIterator = unit.newOrderStatus.erase(newOrderStatusIterator);
    }
}
//cys no use
void TDEngineBitstamp::set_reader_thread()
{
    ITDEngine::set_reader_thread();

    KF_LOG_INFO(logger, "[set_reader_thread] rest_thread start on TDEngineBitstamp::loop");
    rest_thread = ThreadPtr(new std::thread(boost::bind(&TDEngineBitstamp::loop, this)));

    KF_LOG_INFO(logger, "[set_reader_thread] orderaction_timeout_thread start on TDEngineBitstamp::loopOrderActionNoResponseTimeOut");
    orderaction_timeout_thread = ThreadPtr(new std::thread(boost::bind(&TDEngineBitstamp::loopOrderActionNoResponseTimeOut, this)));
}
//cys no use
void TDEngineBitstamp::loop()
{
    KF_LOG_INFO(logger, "[loop] (isRunning) " << isRunning);
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


void TDEngineBitstamp::loopOrderActionNoResponseTimeOut()
{
    KF_LOG_INFO(logger, "[loopOrderActionNoResponseTimeOut] (isRunning) " << isRunning);
    while(isRunning)
    {
        orderActionNoResponseTimeOut();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void TDEngineBitstamp::orderActionNoResponseTimeOut(){
    //    KF_LOG_DEBUG(logger, "[orderActionNoResponseTimeOut]");
    int errorId = 100;
    std::string errorMsg = "OrderAction has none response for a long time(" + std::to_string(orderaction_max_waiting_seconds) + " s), please send OrderAction again";

    std::lock_guard<std::mutex> guard_mutex_order_action(*mutex_orderaction_waiting_response);

    int64_t currentNano = getTimestamp();
    int64_t timeBeforeNano = currentNano - orderaction_max_waiting_seconds * 1000;
    //    KF_LOG_DEBUG(logger, "[orderActionNoResponseTimeOut] (currentNano)" << currentNano << " (timeBeforeNano)" << timeBeforeNano);
    std::map<std::string, OrderActionSentTime>::iterator itr;
    for(itr = remoteOrderIdOrderActionSentTime.begin(); itr != remoteOrderIdOrderActionSentTime.end();)
    {
        if(itr->second.sentNameTime < timeBeforeNano)
        {
            KF_LOG_DEBUG(logger, "[orderActionNoResponseTimeOut] (remoteOrderIdOrderActionSentTime.erase remoteOrderId)" << itr->first );
            on_rsp_order_action(&itr->second.data, itr->second.requestId, errorId, errorMsg.c_str());
            itr = remoteOrderIdOrderActionSentTime.erase(itr);
        } else {
            ++itr;
        }
    }
    //    KF_LOG_DEBUG(logger, "[orderActionNoResponseTimeOut] (remoteOrderIdOrderActionSentTime.size)" << remoteOrderIdOrderActionSentTime.size());
}

void TDEngineBitstamp::printResponse(const Document& d){
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    KF_LOG_INFO(logger, "[printResponse] ok (text) " << buffer.GetString());
}

void TDEngineBitstamp::getResponse(int http_status_code, std::string responseText, std::string errorMsg, Document& json)
{
    if(http_status_code >= HTTP_RESPONSE_OK && http_status_code <= 299)
    {
        json.Parse(responseText.c_str());
    } else if(http_status_code == 0)
    {
        json.SetObject();
        Document::AllocatorType& allocator = json.GetAllocator();
        int errorId = 1;
        json.AddMember("code", errorId, allocator);
        KF_LOG_INFO(logger, "[getResponse] (errorMsg)" << errorMsg);
        rapidjson::Value val;
        val.SetString(errorMsg.c_str(), errorMsg.length(), allocator);
        json.AddMember("msg", val, allocator);
    } else
    {
        Document d;
        d.Parse(responseText.c_str());
        KF_LOG_INFO(logger, "[getResponse] (err) (responseText)" << responseText.c_str());
        json.SetObject();
        Document::AllocatorType& allocator = json.GetAllocator();
        json.AddMember("code", http_status_code, allocator);

        rapidjson::Value val;
        val.SetString(errorMsg.c_str(), errorMsg.length(), allocator);
        json.AddMember("msg", val, allocator);
    }
}

void TDEngineBitstamp::get_account(AccountUnitBitstamp& unit, Document& json)
{
    KF_LOG_INFO(logger, "[get_account]");
    string path="https://www.bitstamp.net/api/v2/balance/";
    int64_t nonce = getTimestamp();
    string nonceStr=std::to_string(nonce);
    string Message = nonceStr + unit.customer_id + unit.api_key;
    std::string signature =  hmac_sha256( unit.secret_key.c_str(), Message.c_str() );
    transform(signature.begin(),signature.end(),signature.begin(),::toupper);
    std::string queryString= "";
    queryString.append( "key=" );
    queryString.append( unit.api_key );
        queryString.append( "&signature=" );
    queryString.append( signature );
        queryString.append( "&nonce=" );
    queryString.append( nonceStr );    

    std::string body1 = queryString;        

    const auto response = Post(path,body1,"",unit);
    json.Parse(response.text.c_str());
    //KF_LOG_INFO(logger, "[get_account] (account info) "<<response.text.c_str());
    return ;
}
std::string TDEngineBitstamp::createInsertOrdertring(string pair,string type,string ordertype,string price,string volume,
        string oflags,string userref){
    string s="";
    s=s+"pair="+pair+"&"+
        "type="+type+"&"+
        "ordertype="+ordertype+"&"+
        "price="+price+"&"+
        "volume="+volume+"&"+
        "userref="+userref;

    return s;
}
void TDEngineBitstamp::send_buylimitorder(AccountUnitBitstamp& unit, string userref, string code,
                        string side, string type, string volume, string price, Document& json){
    KF_LOG_INFO(logger, "[send_buylimitorder]");
    KF_LOG_INFO(logger, "[send_order] (code) "<<code);
    int retry_times = 0;
    cpr::Response response;
    bool should_retry = false;
    do {
        should_retry = false;
        string path = "https://www.bitstamp.net/api/v2/buy/";
        string url = path + code + "/";
        //string postData=createInsertOrdertring(code, side, type, price,volume,"",userref);
        int64_t nonce = getTimestamp();
        string nonceStr=std::to_string(nonce);
        string Message = nonceStr + unit.customer_id + unit.api_key;
        std::string signature =  hmac_sha256( unit.secret_key.c_str(), Message.c_str() );
        transform(signature.begin(),signature.end(),signature.begin(),::toupper);
        std::string queryString= "";
        queryString.append( "key=" );
        queryString.append( unit.api_key );
            queryString.append( "&signature=" );
        queryString.append( signature );
            queryString.append( "&nonce=" );
        queryString.append( nonceStr );
            queryString.append( "&amount=" );
        queryString.append( volume );
            queryString.append( "&price=" );
        queryString.append( price );
           /* queryString.append( "&limit_price=" );
        queryString.append( price );*/     

        std::string body1 = queryString;      

        response = Post(url,body1,"",unit);

        KF_LOG_INFO(logger, "[send_order] (url) " << url << " (response.status_code) " << response.status_code 
                                                  << " (response.error.message) " << response.error.message 
                                                  << " (retry_times)" << retry_times);

        //json.Clear();
        getResponse(response.status_code, response.text, response.error.message, json);
        if(shouldRetry(json)) {
            should_retry = true;
            retry_times++;
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
        }
    } while(should_retry && retry_times < max_rest_retry_times);

    KF_LOG_INFO(logger, "[send_buylimitorder] out_retry (response.status_code) " << response.status_code <<" (response.error.message) " 
                                                                        << response.error.message << " (response.text) " << response.text.c_str() );

    //getResponse(response.status_code, response.text, response.error.message, json);
}
void TDEngineBitstamp::send_buymarketorder(AccountUnitBitstamp& unit, string userref, string code,
                        string side, string type, string volume, string price, Document& json){
    KF_LOG_INFO(logger, "[send_buymarketorder]");
    KF_LOG_INFO(logger, "[send_order] (code) "<<code);
    int retry_times = 0;
    cpr::Response response;
    bool should_retry = false;
    do {
        should_retry = false;
        string path = "https://www.bitstamp.net/api/v2/buy/market/";
        string url = path + code + "/";
        //string postData=createInsertOrdertring(code, side, type, price,volume,"",userref);
        int64_t nonce = getTimestamp();
        string nonceStr=std::to_string(nonce);
        string Message = nonceStr + unit.customer_id + unit.api_key;
        std::string signature =  hmac_sha256( unit.secret_key.c_str(), Message.c_str() );
        transform(signature.begin(),signature.end(),signature.begin(),::toupper);
        std::string queryString= "";
        queryString.append( "key=" );
        queryString.append( unit.api_key );
            queryString.append( "&signature=" );
        queryString.append( signature );
            queryString.append( "&nonce=" );
        queryString.append( nonceStr );
            queryString.append( "&amount=" );
        queryString.append( volume );

        std::string body1 = queryString;      

        response = Post(url,body1,"",unit);

        KF_LOG_INFO(logger, "[send_order] (url) " << url << " (response.status_code) " << response.status_code 
                                                  << " (response.error.message) " << response.error.message 
                                                  << " (retry_times)" << retry_times);

        //json.Clear();
        getResponse(response.status_code, response.text, response.error.message, json);
        if(shouldRetry(json)) {
            should_retry = true;
            retry_times++;
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
        }
    } while(should_retry && retry_times < max_rest_retry_times);

    KF_LOG_INFO(logger, "[send_buymarketorder] out_retry (response.status_code) " << response.status_code <<" (response.error.message) " 
                                                                        << response.error.message << " (response.text) " << response.text.c_str() );

    //getResponse(response.status_code, response.text, response.error.message, json);
}
void TDEngineBitstamp::send_selllimitorder(AccountUnitBitstamp& unit, string userref, string code,
                        string side, string type, string volume, string price, Document& json){
    KF_LOG_INFO(logger, "[send_selllimitorder]");
    KF_LOG_INFO(logger, "[send_order] (code) "<<code);
    int retry_times = 0;
    cpr::Response response;
    bool should_retry = false;
    do {
        should_retry = false;
        string path = "https://www.bitstamp.net/api/v2/sell/";
        string url = path + code + "/";
        //string postData=createInsertOrdertring(code, side, type, price,volume,"",userref);
        int64_t nonce = getTimestamp();
        string nonceStr=std::to_string(nonce);
        string Message = nonceStr + unit.customer_id + unit.api_key;
        std::string signature =  hmac_sha256( unit.secret_key.c_str(), Message.c_str() );
        transform(signature.begin(),signature.end(),signature.begin(),::toupper);
        std::string queryString= "";
        queryString.append( "key=" );
        queryString.append( unit.api_key );
            queryString.append( "&signature=" );
        queryString.append( signature );
            queryString.append( "&nonce=" );
        queryString.append( nonceStr );
            queryString.append( "&amount=" );
        queryString.append( volume );
            queryString.append( "&price=" );
        queryString.append( price );
           /* queryString.append( "&limit_price=" );
        queryString.append( price ); */    

        std::string body1 = queryString;      

        response = Post(url,body1,"",unit);

        KF_LOG_INFO(logger, "[send_selllimitorder] (url) " << url << " (response.status_code) " << response.status_code 
                                                  << " (response.error.message) " << response.error.message 
                                                  << " (retry_times)" << retry_times);

        //json.Clear();
        getResponse(response.status_code, response.text, response.error.message, json);
        if(shouldRetry(json)) {
            should_retry = true;
            retry_times++;
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
        }
    } while(should_retry && retry_times < max_rest_retry_times);

    KF_LOG_INFO(logger, "[send_order] out_retry (response.status_code) " << response.status_code <<" (response.error.message) " 
                                                                        << response.error.message << " (response.text) " << response.text.c_str() );

    //getResponse(response.status_code, response.text, response.error.message, json);
}
void TDEngineBitstamp::send_sellmarketorder(AccountUnitBitstamp& unit, string userref, string code,
                        string side, string type, string volume, string price, Document& json){
    KF_LOG_INFO(logger, "[send_sellmarketorder]");
    KF_LOG_INFO(logger, "[send_order] (code) "<<code);
    int retry_times = 0;
    cpr::Response response;
    bool should_retry = false;
    do {
        should_retry = false;
        string path = "https://www.bitstamp.net/api/v2/sell/market/";
        string url = path + code + "/";
        //string postData=createInsertOrdertring(code, side, type, price,volume,"",userref);
        int64_t nonce = getTimestamp();
        string nonceStr=std::to_string(nonce);
        string Message = nonceStr + unit.customer_id + unit.api_key;
        std::string signature =  hmac_sha256( unit.secret_key.c_str(), Message.c_str() );
        transform(signature.begin(),signature.end(),signature.begin(),::toupper);
        std::string queryString= "";
        queryString.append( "key=" );
        queryString.append( unit.api_key );
            queryString.append( "&signature=" );
        queryString.append( signature );
            queryString.append( "&nonce=" );
        queryString.append( nonceStr );
            queryString.append( "&amount=" );
        queryString.append( volume );

        std::string body1 = queryString;      

        response = Post(url,body1,"",unit);

        KF_LOG_INFO(logger, "[send_sellmarketorder] (url) " << url << " (response.status_code) " << response.status_code 
                                                  << " (response.error.message) " << response.error.message 
                                                  << " (retry_times)" << retry_times);

        //json.Clear();
        getResponse(response.status_code, response.text, response.error.message, json);
        if(shouldRetry(json)) {
            should_retry = true;
            retry_times++;
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
        }
    } while(should_retry && retry_times < max_rest_retry_times);

    KF_LOG_INFO(logger, "[send_sellmarketorder] out_retry (response.status_code) " << response.status_code <<" (response.error.message) " 
                                                                        << response.error.message << " (response.text) " << response.text.c_str() );

    //getResponse(response.status_code, response.text, response.error.message, json);
}
void TDEngineBitstamp::order_status(AccountUnitBitstamp& unit, std::string code, std::string orderId, Document& json)
{
    KF_LOG_INFO(logger, "[order_status]");

    int retry_times = 0;
    cpr::Response response;
    bool should_retry = false;
    do {
        should_retry = false;
        std::string path="https://www.bitstamp.net/api/order_status/";
        int64_t nonce = getTimestamp();
        string nonceStr=std::to_string(nonce);
        string Message = nonceStr + unit.customer_id + unit.api_key;
        std::string signature =  hmac_sha256( unit.secret_key.c_str(), Message.c_str() );
        transform(signature.begin(),signature.end(),signature.begin(),::toupper);
        std::string queryString= "";
        queryString.append( "key=" );
        queryString.append( unit.api_key );
            queryString.append( "&signature=" );
        queryString.append( signature );
            queryString.append( "&nonce=" );
        queryString.append( nonceStr );
            queryString.append( "&id=" );
        queryString.append( orderId );       

        std::string body1 = queryString; 

        response = Post(path,body1,"",unit);

        //json.Clear();
        getResponse(response.status_code, response.text, response.error.message, json);
        //has error and find the 'error setting certificate verify locations' error, should retry
        if(shouldRetry(json)) {
            should_retry = true;
            retry_times++;
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
        }
    } while(should_retry && retry_times < max_rest_retry_times);


    KF_LOG_INFO(logger, "[order_status] out_retry " << retry_times << " (response.status_code) " << response.status_code <<
                                                    " (response.error.message) " << response.error.message <<
                                                    " (response.text) " << response.text.c_str() );

    //getResponse(response.status_code, response.text, response.error.message, json);
}
bool TDEngineBitstamp::shouldRetry(Document& doc)
{
    bool ret = false;
    int errLen = 0;
    /*if(doc.HasMember("error"))
    {
        errLen = doc["error"].Size();
    }*/
    KF_LOG_INFO(logger, "[shouldRetry] ret = " << ret << ", errLen = " << errLen);
    return ret;
}

void TDEngineBitstamp::cancel_order(AccountUnitBitstamp& unit, std::string code, std::string orderId, Document& json)
{
    KF_LOG_INFO(logger, "[]");

    int retry_times = 0;
    cpr::Response response;
    bool should_retry = false;
    do {
        should_retry = false;
        std::string path="https://www.bitstamp.net/api/v2/cancel_order/";
        int64_t nonce = getTimestamp();
        string nonceStr=std::to_string(nonce);
        string Message = nonceStr + unit.customer_id + unit.api_key;
        std::string signature =  hmac_sha256( unit.secret_key.c_str(), Message.c_str() );
        transform(signature.begin(),signature.end(),signature.begin(),::toupper);
        std::string queryString= "";
        queryString.append( "key=" );
        queryString.append( unit.api_key );
            queryString.append( "&signature=" );
        queryString.append( signature );
            queryString.append( "&nonce=" );
        queryString.append( nonceStr );
            queryString.append( "&id=" );
        queryString.append( orderId );       

        std::string body1 = queryString; 

        response = Post(path,body1,"",unit);

        //json.Clear();
        getResponse(response.status_code, response.text, response.error.message, json);
        //has error and find the 'error setting certificate verify locations' error, should retry
        if(shouldRetry(json)) {
            should_retry = true;
            retry_times++;
            std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_milliseconds));
        }
    } while(should_retry && retry_times < max_rest_retry_times);


    KF_LOG_INFO(logger, "[cancel_order] out_retry " << retry_times << " (response.status_code) " << response.status_code <<
                                                    " (response.error.message) " << response.error.message <<
                                                    " (response.text) " << response.text.c_str() );

    //getResponse(response.status_code, response.text, response.error.message, json);
}
void TDEngineBitstamp::cancel_all_orders(AccountUnitBitstamp& unit, Document& json)
{
    KF_LOG_INFO(logger, "[cancel_all_orders]");
    string path="https://www.bitstamp.net/api/cancel_all_orders/";
    int64_t nonce = getTimestamp();
    string nonceStr=std::to_string(nonce);
    string Message = nonceStr + unit.customer_id + unit.api_key;
    std::string signature =  hmac_sha256( unit.secret_key.c_str(), Message.c_str() );
    transform(signature.begin(),signature.end(),signature.begin(),::toupper);
    std::string queryString= "";
    queryString.append( "key=" );
    queryString.append( unit.api_key );
        queryString.append( "&signature=" );
    queryString.append( signature );
        queryString.append( "&nonce=" );
    queryString.append( nonceStr );    

    std::string body1 = queryString;        

    const auto response = Post(path,body1,"",unit);
    json.Parse(response.text.c_str());
    //KF_LOG_INFO(logger, "[get_account] (account info) "<<response.text.c_str());
    return ;
}

void TDEngineBitstamp::query_order(AccountUnitBitstamp& unit, std::string code, std::string orderId, Document& json)
{
    KF_LOG_INFO(logger, "[query_order] start");
    //Bitstamp查询订单详情
    string getPath = "https://www.bitstamp.net/api/order_status/";
    int64_t nonce = getTimestamp();
    string nonceStr=std::to_string(nonce);
    string Message = nonceStr + unit.customer_id + unit.api_key;
    std::string signature =  hmac_sha256( unit.secret_key.c_str(), Message.c_str() );
    transform(signature.begin(),signature.end(),signature.begin(),::toupper);
    std::string queryString= "";
    queryString.append( "key=" );
    queryString.append( unit.api_key );
        queryString.append( "&signature=" );
    queryString.append( signature );
        queryString.append( "&nonce=" );
    queryString.append( nonceStr );    
        queryString.append( "&id=" );
    queryString.append( orderId );    
    std::string body1 = queryString;        

    auto response = Post(getPath,body1,"",unit);
    json.Parse(response.text.c_str());
    KF_LOG_INFO(logger, "[query_order] end");
    return;
}
void TDEngineBitstamp::orderIsCanceled(AccountUnitBitstamp& unit, LFRtnOrderField* rtn_order){
    rtn_order->OrderStatus = LF_CHAR_Canceled;
    //累计成交数量
    //rtn_order.VolumeTraded;
    //剩余未成交数量
    //rtn_order->VolumeTotal = rtn_order.VolumeTotalOriginal-rtn_order->VolumeTraded;
    on_rtn_order(rtn_order);
    raw_writer->write_frame(&(*rtn_order), sizeof(LFRtnOrderField),source_id, MSG_TYPE_LF_RTN_TRADE_BITSTAMP,1, 
            (rtn_order->RequestID > 0) ? rtn_order->RequestID: -1);
}
void TDEngineBitstamp::handlerResponseOrderStatus(AccountUnitBitstamp& unit, std::vector<PendingOrderStatus>::iterator itr,
         ResponsedOrderStatus& responsedOrderStatus)
{
    KF_LOG_INFO(logger, "[handlerResponseOrderStatus]");
    LfOrderStatusType orderStatus = responsedOrderStatus.OrderStatus;
        /*if(responsedOrderStatus.VolumeTraded == responsedOrderStatus.volume){
            orderStatus = LF_CHAR_AllTraded;
        }else if(responsedOrderStatus.VolumeTraded == 0){
            orderStatus = LF_CHAR_NotTouched;
        }else if(responsedOrderStatus.VolumeTraded < responsedOrderStatus.volume){
            orderStatus = LF_CHAR_PartTradedQueueing;
        }*/
                    
    
    if(orderStatus == itr->rtn_order.OrderStatus&&responsedOrderStatus.VolumeTraded == itr->rtn_order.VolumeTraded){//no change
        KF_LOG_INFO(logger,"[handlerResponseOrderStatus] order status not change, return nothing.");
        return;
    }
    itr->rtn_order.OrderStatus = orderStatus;
    //单次成交量
    uint64_t singleVolume = responsedOrderStatus.VolumeTraded - itr->rtn_order.VolumeTraded;
    //单次成交价
    double oldAmount = itr->rtn_order.VolumeTraded/(scale_offset*1.0) * itr->averagePrice/(scale_offset*1.0);
    double newAmount = responsedOrderStatus.VolumeTraded/(scale_offset*1.0) * responsedOrderStatus.averagePrice/(scale_offset*1.0);
    double singlePrice = newAmount - oldAmount;
    uint64_t oldVolumeTraded = itr->rtn_order.VolumeTraded;
    //累计成交数量
    itr->rtn_order.VolumeTraded=responsedOrderStatus.VolumeTraded;
    //剩余未成交数量
    itr->rtn_order.VolumeTotal = itr->rtn_order.VolumeTotalOriginal-itr->rtn_order.VolumeTraded;
    itr->averagePrice = responsedOrderStatus.averagePrice;
    on_rtn_order(&(itr->rtn_order));
    raw_writer->write_frame(&(itr->rtn_order), sizeof(LFRtnOrderField),source_id, MSG_TYPE_LF_RTN_TRADE_BITSTAMP,1, (itr->rtn_order.RequestID > 0) ? itr->rtn_order.RequestID: -1);

    if(oldVolumeTraded!=itr->rtn_order.VolumeTraded){
        //send OnRtnTrade
        LFRtnTradeField rtn_trade;
        memset(&rtn_trade, 0, sizeof(LFRtnTradeField));
        strcpy(rtn_trade.ExchangeID, "bitstamp");
        strncpy(rtn_trade.UserID, unit.api_key.c_str(), 16);
        strncpy(rtn_trade.InstrumentID, itr->rtn_order.InstrumentID, 31);
        strncpy(rtn_trade.OrderRef, itr->rtn_order.OrderRef, 13);
        rtn_trade.Direction = itr->rtn_order.Direction;
        //单次成交数量
        rtn_trade.Volume = singleVolume;
        //单次成交价格
        rtn_trade.Price = std::round(singlePrice*scale_offset);
        strncpy(rtn_trade.OrderSysID,itr->rtn_order.BusinessUnit,31);
        on_rtn_trade(&rtn_trade);

        raw_writer->write_frame(&rtn_trade, sizeof(LFRtnTradeField),
            source_id, MSG_TYPE_LF_RTN_TRADE_BITSTAMP, 1, -1);

        KF_LOG_INFO(logger, "[on_rtn_trade 1] (InstrumentID)" << rtn_trade.InstrumentID << "(Direction)" << rtn_trade.Direction
                << "(Volume)" << rtn_trade.Volume << "(Price)" <<  rtn_trade.Price);
    }

}

std::string TDEngineBitstamp::parseJsonToString(Document &d){
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    return buffer.GetString();
}


inline int64_t TDEngineBitstamp::getTimestamp(){
    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return timestamp;
}

void TDEngineBitstamp::genUniqueKey(){
    struct tm cur_time = getCurLocalTime();
    //SSMMHHDDN
    char key[11]{0};
    snprintf((char*)key, 11, "%02d%02d%02d%02d%1s", cur_time.tm_sec, cur_time.tm_min, cur_time.tm_hour, cur_time.tm_mday, m_engineIndex.c_str());
    m_uniqueKey = key;
}
//clientid =  m_uniqueKey+orderRef
std::string TDEngineBitstamp::genClinetid(const std::string &orderRef){
    static int nIndex = 0;
    return m_uniqueKey + orderRef + std::to_string(nIndex++);
}

#define GBK2UTF8(msg) kungfu::yijinjing::gbk2utf8(string(msg))
BOOST_PYTHON_MODULE(libbitstamptd){
    using namespace boost::python;
    class_<TDEngineBitstamp, boost::shared_ptr<TDEngineBitstamp> >("Engine")
     .def(init<>())
        .def("init", &TDEngineBitstamp::initialize)
        .def("start", &TDEngineBitstamp::start)
        .def("stop", &TDEngineBitstamp::stop)
        .def("logout", &TDEngineBitstamp::logout)
        .def("wait_for_stop", &TDEngineBitstamp::wait_for_stop);
}
