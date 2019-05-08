#include "MDEnginePoloniex.h"
#include "TypeConvert.hpp"
#include "Timer.h"
#include "longfist/LFUtils.h"
#include "longfist/LFDataStruct.h"

#include <writer.h>
#include <stringbuffer.h>
#include <document.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <assert.h>
#include <string>
#include <cpr/cpr.h>
#include <chrono>


using cpr::Get;
using cpr::Url;
using cpr::Parameters;
using cpr::Payload;
using cpr::Post;

using rapidjson::Document;
using rapidjson::SizeType;
using rapidjson::Value;
using rapidjson::Writer;
using rapidjson::StringBuffer;
using std::string;
using std::to_string;
using std::stod;
using std::stoi;

USING_WC_NAMESPACE

static MDEnginePoloniex* global_md = nullptr;

static int ws_service_cb( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{

    switch( reason )
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
        {
            lws_callback_on_writable( wsi );
            break;
        }
        case LWS_CALLBACK_PROTOCOL_INIT:
        {
            break;
        }
        case LWS_CALLBACK_CLIENT_RECEIVE:
        {
            if(global_md)
            {
                global_md->on_lws_data(wsi, (const char*)in, len);
            }
            break;
        }
        case LWS_CALLBACK_CLIENT_CLOSED:
        {
            if(global_md) {
                std::cout << "3.1415926 LWS_CALLBACK_CLIENT_CLOSED 2,  (call on_lws_connection_error)  reason = " << reason << std::endl;
                global_md->on_lws_connection_error(wsi);
            }
            break;
        }
        case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
        {
            break;
        }
        case LWS_CALLBACK_CLIENT_WRITEABLE:
        {
            if(global_md)
            {
                global_md->lws_write_subscribe(wsi);
            }
            break;
        }
	    case LWS_CALLBACK_TIMER:
        {
            break;
	    }
        case LWS_CALLBACK_CLOSED:
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        {
            std::cout << "3.1415926 LWS_CALLBACK_CLOSED/LWS_CALLBACK_CLIENT_CONNECTION_ERROR writeable, reason = " << reason << std::endl;
            if(global_md)
            {
                global_md->on_lws_connection_error(wsi);
            }
            break;
        }
        default:
            break;
    }

    return 0;
}

static struct lws_protocols protocols[] =
{
        {
                "md-poloniex",
                ws_service_cb,
                        0,
                            65536,
        },
        { NULL, NULL, 0, 0 } /* terminator */
};


MDEnginePoloniex::MDEnginePoloniex(): IMDEngine(SOURCE_BITFINEX)
{
    logger = yijinjing::KfLog::getLogger("MdEngine.Poloniex");
}

void MDEnginePoloniex::load(const json& j_config)
{
    book_depth_count = j_config["book_depth_count"].get<int>();
    trade_count = j_config["trade_count"].get<int>();
    rest_get_interval_ms = j_config["rest_get_interval_ms"].get<int>();
    KF_LOG_INFO(logger, "MDEnginePoloniex:: rest_get_interval_ms: " << rest_get_interval_ms);


    coinPairWhiteList.ReadWhiteLists(j_config, "whiteLists");
    coinPairWhiteList.Debug_print();

    makeWebsocketSubscribeJsonString();
    debug_print(websocketSubscribeJsonString);

    //display usage:
    if(coinPairWhiteList.Size() == 0) {
        KF_LOG_ERROR(logger, "MDEnginePoloniex::lws_write_subscribe: subscribeCoinBaseQuote is empty. please add whiteLists in kungfu.json like this :");
        KF_LOG_ERROR(logger, "\"whiteLists\":{");
        KF_LOG_ERROR(logger, "    \"strategy_coinpair(base_quote)\": \"exchange_coinpair\",");
        KF_LOG_ERROR(logger, "    \"btc_usdt\": \"tBTCUSDT\",");
        KF_LOG_ERROR(logger, "     \"etc_eth\": \"tETCETH\"");
        KF_LOG_ERROR(logger, "},");
    }

    KF_LOG_INFO(logger, "MDEnginePoloniex::load:  book_depth_count: "
            << book_depth_count << " trade_count: " << trade_count << " rest_get_interval_ms: " << rest_get_interval_ms);
}

void MDEnginePoloniex::makeWebsocketSubscribeJsonString()
{
    std::unordered_map<std::string, std::string>::iterator map_itr;
    map_itr = coinPairWhiteList.GetKeyIsStrategyCoinpairWhiteList().begin();
    while(map_itr != coinPairWhiteList.GetKeyIsStrategyCoinpairWhiteList().end()) {
        KF_LOG_DEBUG(logger, "[makeWebsocketSubscribeJsonString] keyIsExchangeSideWhiteList (strategy_coinpair) " << map_itr->first << " (exchange_coinpair) "<< map_itr->second);

        std::string jsonBookString = createBookJsonString(map_itr->second);
        websocketSubscribeJsonString.push_back(jsonBookString);

        map_itr++;
    }
}

void MDEnginePoloniex::debug_print(std::vector<std::string> &subJsonString)
{
    size_t count = subJsonString.size();
    KF_LOG_INFO(logger, "[debug_print] websocketSubscribeJsonString (count) " << count);

    for (size_t i = 0; i < count; i++)
    {
        KF_LOG_INFO(logger, "[debug_print] websocketSubscribeJsonString (subJsonString) " << subJsonString[i]);
    }
}

void MDEnginePoloniex::connect(long timeout_nsec)
{
    KF_LOG_INFO(logger, "MDEnginePoloniex::connect:");
    connected = true;
}

void MDEnginePoloniex::login(long timeout_nsec) {
    KF_LOG_INFO(logger, "MDEnginePoloniex::login:");
    global_md = this;

    if (context == NULL) {
        struct lws_context_creation_info info;
        memset( &info, 0, sizeof(info) );

        info.port = CONTEXT_PORT_NO_LISTEN;
        info.protocols = protocols;
        info.iface = NULL;
        info.ssl_cert_filepath = NULL;
        info.ssl_private_key_filepath = NULL;
        info.extensions = NULL;
        info.gid = -1;
        info.uid = -1;
        info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        info.max_http_header_pool = 1024;
        info.fd_limit_per_thread = 1024;
        info.ws_ping_pong_interval = 10;
        info.ka_time = 10;
        info.ka_probes = 10;
        info.ka_interval = 10;

        context = lws_create_context( &info );
        KF_LOG_INFO(logger, "MDEnginePoloniex::login: context created.");
    }

    if (context == NULL) {
        KF_LOG_ERROR(logger, "MDEnginePoloniex::login: context is NULL. return");
        return;
    }

    int logs = LLL_ERR | LLL_DEBUG | LLL_WARN;
    lws_set_log_level(logs, NULL);

    struct lws_client_connect_info ccinfo = {0};

    static std::string host  = "api2.poloniex.com";
    static std::string path = "";
    static int port = 443;

    ccinfo.context 	= context;
    ccinfo.address 	= host.c_str();
    ccinfo.port 	= port;
    ccinfo.path 	= path.c_str();
    ccinfo.host 	= host.c_str();
    ccinfo.origin 	= host.c_str();
    ccinfo.ietf_version_or_minus_one = -1;
    ccinfo.protocol = protocols[0].name;
    ccinfo.ssl_connection = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED | LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;

    struct lws* wsi = lws_client_connect_via_info(&ccinfo);
    KF_LOG_INFO(logger, "MDEnginePoloniex::login: Connecting to " <<  ccinfo.host << ":" << ccinfo.port << ":" << ccinfo.path);

    if (wsi == NULL) {
        KF_LOG_ERROR(logger, "MDEnginePoloniex::login: wsi create error.");
        return;
    }
    KF_LOG_INFO(logger, "MDEnginePoloniex::login: wsi create success.");

    logged_in = true;
}

void MDEnginePoloniex::set_reader_thread()
{
    IMDEngine::set_reader_thread();
    rest_thread = ThreadPtr(new std::thread(boost::bind(&MDEnginePoloniex::loop, this)));
}

void MDEnginePoloniex::logout()
{
    KF_LOG_INFO(logger, "MDEnginePoloniex::logout:");
}

void MDEnginePoloniex::release_api()
{
    KF_LOG_INFO(logger, "MDEnginePoloniex::release_api:");
}

void MDEnginePoloniex::subscribeMarketData(const vector<string>& instruments, const vector<string>& markets)
{
    KF_LOG_INFO(logger, "MDEnginePoloniex::subscribeMarketData:");
}

int MDEnginePoloniex::lws_write_subscribe(struct lws* conn)
{
    KF_LOG_INFO(logger, "MDEnginePoloniex::lws_write_subscribe: (subscribe_index)" << subscribe_index);

    if(websocketSubscribeJsonString.size() == 0) return 0;
    //sub depth
    if(subscribe_index >= websocketSubscribeJsonString.size())
    {
        //subscribe_index = 0;
        KF_LOG_INFO(logger, "MDEnginePoloniex::lws_write_subscribe: (none reset subscribe_index = 0, just return 0)");
	    return 0;
    }

    unsigned char msg[512];
    memset(&msg[LWS_PRE], 0, 512-LWS_PRE);

    std::string jsonString = websocketSubscribeJsonString[subscribe_index++];

    KF_LOG_INFO(logger, "MDEnginePoloniex::lws_write_subscribe: " << jsonString.c_str() << " ,after ++, (subscribe_index)" << subscribe_index);
    int length = jsonString.length();

    strncpy((char *)msg+LWS_PRE, jsonString.c_str(), length);
    int ret = lws_write(conn, &msg[LWS_PRE], length,LWS_WRITE_TEXT);

    if(subscribe_index < websocketSubscribeJsonString.size())
    {
        lws_callback_on_writable( conn );
        KF_LOG_INFO(logger, "MDEnginePoloniex::lws_write_subscribe: (subscribe_index < websocketSubscribeJsonString) call lws_callback_on_writable");
    }

    return ret;
}

void MDEnginePoloniex::on_lws_data(struct lws* conn, const char* data, size_t len)
{
    KF_LOG_INFO(logger, "MDEnginePoloniex::on_lws_data: " << data);
    Document json;
    json.Parse(data);

    if(json.HasParseError()) {
        KF_LOG_ERROR(logger, "MDEnginePoloniex::on_lws_data. parse json error: " << data);
        return;
    }

    if(json.IsArray()){
        int channelId = json.GetArray()[0].GetInt();
        //get initialization infomation
        if(strcmp(json.GetArray()[2].GetArray()[0].GetArray()[0].GetString(),"i")==0){
            KF_LOG_INFO(logger,"MDEnginePology::on_lws_data: getInfo: inistial");
            GetINitializationInfomation(json,channelId,true);
        }
        else{
            KF_LOG_INFO(logger,"MDEnginePology::on_lws_data: getInfo: getchange");
            GetINitializationInfomation(json,channelId,false);
        }



    }


}


void MDEnginePoloniex::GetINitializationInfomation(Document& json, int channlId, bool isInistial)
{
    KF_LOG_INFO(logger,"MDEnginePoloniex::GetINitializationInfomation");
    std::string ticker;

    if(isInistial){
        ticker = json.GetArray()[2].GetArray()[0].GetArray()[1]["currencyPair"].GetString();
        if(ticker.length()==0) return;

        KF_LOG_INFO(logger,"MDEnginePoloniex::GetINitializationInfomation"<<ticker);

        SubscribeChannel newChannel;
        newChannel.channelId = channlId;
        newChannel.exchange_coinpair = ticker;
        websocketSubscribeChannel.push_back(newChannel);
        debug_print(websocketSubscribeChannel);

        LFPriceBook20Field md;
        memset(&md, 0, sizeof(md));
        if(priceBook20Assembler.Assembler(ticker, md)) {
            strcpy(md.ExchangeID, "poloniex");

            KF_LOG_INFO(logger, "MDEnginePoloniex::onDepth: on_price_book_update");
            on_price_book_update(&md);
        }
    }
    else{
        SubscribeChannel channel = findByChannelID(channlId);
        ticker = coinPairWhiteList.GetKeyByValue(channel.exchange_coinpair);
        if(ticker.length()==0) return;

        int len = json.GetArray()[2].Size();
        for(int i = 0; i < len; i++){
            if(strcmp(json.GetArray()[2].GetArray()[i].GetArray()[0].GetString(),"o")==0){
                KF_LOG_INFO(logger,"MDEnginePoloniex::GetINitializationInfomation: operation : o");
                int isBookBuy = json.GetArray()[2].GetArray()[i].GetArray()[1].GetInt();
                int64_t price = std::round(json.GetArray()[2].GetArray()[i].GetArray()[2].GetDouble()*scale_offset);
                uint64_t amount = std::round(json.GetArray()[2].GetArray()[i].GetArray()[3].GetDouble()*scale_offset);
                if(amount>0){
                    if(isBookBuy==1){
                        KF_LOG_INFO(logger,"MDEnginePoloniex::GetINitializationInfomation: operation : o : buy & amount!=0");
                        priceBook20Assembler.UpdateBidPrice(ticker,price,amount);
                    }
                    else {
                        KF_LOG_INFO(logger,"MDEnginePoloniex::GetINitializationInfomation: operation : o : sell & amount!=0");
                        priceBook20Assembler.UpdateAskPrice(ticker,price,amount);
                    }
                }
                else if(amount==0){
                    if(isBookBuy==1){
                        KF_LOG_INFO(logger,"MDEnginePoloniex::GetINitializationInfomation: operation : o : buy & amount==0");
                        priceBook20Assembler.EraseBidPrice(ticker,price);
                    }
                    else {
                        KF_LOG_INFO(logger,"MDEnginePoloniex::GetINitializationInfomation: operation : o : sell & amount==0");
                        priceBook20Assembler.EraseAskPrice(ticker,price);
                    }
                }
                else KF_LOG_INFO(logger,"MDEnginePoloniex::GetINitializationInfomation: operation : o : false");
                LFPriceBook20Field md;
                memset(&md, 0, sizeof(md));
                if(priceBook20Assembler.Assembler(ticker, md)) {
                    strcpy(md.ExchangeID, "poloniex");

                    KF_LOG_INFO(logger, "MDEnginePoloniex::onDepth: on_price_book_update");
                    on_price_book_update(&md);
                }
            }
            else if(strcmp(json.GetArray()[2].GetArray()[i].GetArray()[0].GetString(),"t")==0){
                KF_LOG_INFO(logger,"MDEnginePoloniex::GetINitializationInfomation: operation : t");
                LFL2TradeField trade;
                memset(&trade, 0, sizeof(trade));
                strcpy(trade.InstrumentID, ticker.c_str());
                strcpy(trade.ExchangeID, "poloniex");
                trade.Price = std::round(json.GetArray()[2].GetArray()[i].GetArray()[3].GetDouble()*scale_offset);
                int isTradeBuy = json.GetArray()[2].GetArray()[i].GetArray()[2].GetInt();
                uint64_t amount = std::round(json.GetArray()[2].GetArray()[i].GetArray()[4].GetDouble()*scale_offset);
                trade.Volume = amount;
                trade.OrderBSFlag[0] = isTradeBuy == 1 ? 'B' : 'S';
                KF_LOG_INFO(logger, "MDEnginePoloniex::[onTrade] (ticker)" << ticker <<
                                                                           " (Price)" << trade.Price <<
                                                                           " (trade.Volume)" << trade.Volume);
                on_trade(&trade);
            }
        }
    }
}


void MDEnginePoloniex::on_lws_connection_error(struct lws* conn)
{
    KF_LOG_ERROR(logger, "MDEnginePoloniex::on_lws_connection_error.");
    //market logged_in false;
    logged_in = false;
    KF_LOG_ERROR(logger, "MDEnginePoloniex::on_lws_connection_error. login again.");
    //clear the price book, the new websocket will give 200 depth on the first connect, it will make a new price book
    priceBook20Assembler.clearPriceBook();
    //no use it
    long timeout_nsec = 0;
    //reset sub
    subscribe_index = 0;

    login(timeout_nsec);
}


int64_t MDEnginePoloniex::getTimestamp()
{
    long long timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return timestamp;
}



/*
 * #1
 * {
   "event":"info",
   "code": CODE,
   "msg": MSG
}
#2
 {
   "event": "info",
   "version":  VERSION,
   "platform": {
      "status": 1
   }
}
 * */
void MDEnginePoloniex::onInfo(Document& json)
{
    KF_LOG_INFO(logger, "MDEnginePoloniex::onInfo: " << parseJsonToString(json));
}




void MDEnginePoloniex::debug_print(std::vector<SubscribeChannel> &websocketSubscribeChannel)
{
    size_t count = websocketSubscribeChannel.size();
    KF_LOG_INFO(logger, "[debug_print] websocketSubscribeChannel (count) " << count);

    for (size_t i = 0; i < count; i++)
    {
        KF_LOG_INFO(logger,  " (exchange_coinpair)" << websocketSubscribeChannel[i].exchange_coinpair <<
                            " (channelId)" << websocketSubscribeChannel[i].channelId);
    }
}

SubscribeChannel MDEnginePoloniex::findByChannelID(int channelId)
{
    size_t count = websocketSubscribeChannel.size();

    for (size_t i = 0; i < count; i++)
    {
        if(channelId == websocketSubscribeChannel[i].channelId) {
            return websocketSubscribeChannel[i];
        }
    }
    return EMPTY_CHANNEL;
}


//[1,[[279619183,1534151022575,0.05404775,6485.1],[279619171,1534151022010,-1.04,6485],[279619170,1534151021847,-0.02211732,6485],......]
//[1,"te",[279619192,1534151024181,-0.05678467,6485]]
void MDEnginePoloniex::onTrade(SubscribeChannel &channel, Document& json)
{
    KF_LOG_INFO(logger, "MDEnginePoloniex::onTrade: (symbol) " << channel.exchange_coinpair);

    std::string ticker = coinPairWhiteList.GetKeyByValue(channel.exchange_coinpair);
    if(ticker.length() == 0) {
        return;
    }

    int size = json.GetArray().Size();
    if(size < 2) return;

    int last_element = size - 1;
    if (json.GetArray()[last_element].IsArray()) {
        int len = json.GetArray()[last_element].Size();
        if (len == 0) return;

        if(json.GetArray()[last_element].GetArray()[0].IsArray())
        {
            /* snapshot
             * [
                  CHANNEL_ID,
                  [
                    [
                      ID,
                      MTS,
                      AMOUNT,
                      PRICE
                    ],
                    ...
                  ]
                ]
                //±AMOUNT	float	How much was bought (positive) or sold (negative).
                //正的是: S     (maker is buyer )
                //负的是: B
                //[1,[[279619183,1534151022575,0.05404775,6485.1],[279619171,1534151022010,-1.04,6485],[279619170,1534151021847,-0.02211732,6485],[279619167,1534151021199,-0.61188115,6485.1],[279619166,1534151019315,-0.22695,6485.1],[279619156,1534151014908,-0.05675262,6485.1],[279619153,1534151013760,0.04885593,6485.2],[279619149,1534151013009,-0.03700977,6485.1],[279619140,1534151009718,-0.3416722,6485.1],[279619135,1534151009007,-0.0099409,6485.1],[279619134,1534151008682,-0.28963734,6485.1],[279619129,1534151007656,-0.00695966,6485.1],[279619128,1534151007443,0.003855,6485.2],[279619123,1534151005539,-0.05533626,6485.1],[279619121,1534151005326,0.05081637,6485.2],[279619102,1534151004043,-0.00737768,6485.1],[279619100,1534151003819,-0.05475973,6485.1],[279619096,1534151002811,-0.05475973,6485.1],[279619080,1534151001414,-0.01828309,6485.1],[279619077,1534151000660,-0.0147,6485.1],[279619066,1534150998307,-0.09175605,6485.1],[279619065,1534150998206,-0.0522174,6485.1],[279619020,1534150994733,0.05436071,6485.2],[279618991,1534150990781,0.0101821,6485.2],[279618946,1534150986977,0.24390946,6485.2],[279618918,1534150986112,0.0203,6485.2],[279618917,1534150986109,0.05562306,6485.2],[279618838,1534150977454,0.03,6485.2],[279618828,1534150976649,0.05351248,6485.2],[279618827,1534150975684,0.02853241,6485.2]]]
             * */
            for (int i = 0; i < len; i++) {
//                KF_LOG_INFO(logger, " (0)" << json.GetArray()[last_element].GetArray()[i].GetArray()[0].GetInt64() );
//                KF_LOG_INFO(logger, " (1)" << json.GetArray()[last_element].GetArray()[i].GetArray()[1].GetInt64() );
//                KF_LOG_INFO(logger, " (2)" << json.GetArray()[last_element].GetArray()[i].GetArray()[2].GetDouble() );
//                KF_LOG_INFO(logger, " (3)" << json.GetArray()[last_element].GetArray()[i].GetArray()[3].GetDouble() );

                LFL2TradeField trade;
                memset(&trade, 0, sizeof(trade));
                strcpy(trade.InstrumentID, ticker.c_str());
                strcpy(trade.ExchangeID, "bitfinex");

                trade.Price = std::round(json.GetArray()[last_element].GetArray()[i].GetArray()[3].GetDouble() * scale_offset);
                double amount = json.GetArray()[last_element].GetArray()[i].GetArray()[2].GetDouble();
                uint64_t volume = 0;
                if(amount < 0) {
                    volume = std::round(-1 * amount * scale_offset);
                } else {
                    volume = std::round( amount * scale_offset);
                }

                trade.Volume = volume;
                trade.OrderBSFlag[0] = amount < 0 ? 'B' : 'S';
                KF_LOG_INFO(logger, "MDEnginePoloniex::[onTrade] (ticker)" << ticker <<
                                                                           " (Price)" << trade.Price <<
                                                                           " (trade.Volume)" << trade.Volume);
                on_trade(&trade);

            }
        } else {
            /*update
             * [
              CHANNEL_ID,
              <"te", "tu">,
              [
                ID,
                MTS,
                AMOUNT,
                PRICE
              ]
            ]
            //±AMOUNT	float	How much was bought (positive) or sold (negative).
            //正的是: S     (maker is buyer )
            //负的是: B
            //[1,"te",[279619192,1534151024181,-0.05678467,6485.1]]
             * */
//            KF_LOG_INFO(logger, " update(0)" << json.GetArray()[last_element].GetArray()[0].GetInt64());
//            KF_LOG_INFO(logger, " update(1)" << json.GetArray()[last_element].GetArray()[1].GetInt64());
//            KF_LOG_INFO(logger, " update(2)"<< json.GetArray()[last_element].GetArray()[2].GetDouble());
//            KF_LOG_INFO(logger, " update(3)"<< json.GetArray()[last_element].GetArray()[3].GetDouble());

            LFL2TradeField trade;
            memset(&trade, 0, sizeof(trade));
            strcpy(trade.InstrumentID, ticker.c_str());
            strcpy(trade.ExchangeID, "bitfinex");

            trade.Price = std::round(json.GetArray()[last_element].GetArray()[3].GetDouble() * scale_offset);
            double amount = json.GetArray()[last_element].GetArray()[2].GetDouble();
            uint64_t volume = 0;
            if(amount < 0) {
                volume = std::round(-1 * amount * scale_offset);
            } else {
                volume = std::round( amount * scale_offset);
            }
            trade.Volume = volume;
            trade.OrderBSFlag[0] = amount < 0 ? 'B' : 'S';
            KF_LOG_INFO(logger, "MDEnginePoloniex::[onTrade] (ticker)" << ticker <<
                                                                       " (Price)" << trade.Price <<
                                                                       " (trade.Volume)" << trade.Volume);
            on_trade(&trade);
        }
    }
}


void MDEnginePoloniex::onBook(SubscribeChannel &channel, Document& json)
{
    KF_LOG_INFO(logger, "MDEnginePoloniex::onBook: (symbol) " << channel.exchange_coinpair);

    std::string ticker = coinPairWhiteList.GetKeyByValue(channel.exchange_coinpair);
    if(ticker.length() == 0) {
        return;
    }

    KF_LOG_INFO(logger, "MDEnginePoloniex::onBook: (ticker) " << ticker);

    int size = json.GetArray().Size();
    int last_element = size - 1;
    if (json.GetArray()[last_element].IsArray()) {
        int len = json.GetArray()[last_element].Size();
        if (len == 0) return;

        if(json.GetArray()[last_element].GetArray()[0].IsArray())
        {
            /* snapshot
             * [
              CHANNEL_ID,
              [
                [
                  PRICE,
                  COUNT,
                  AMOUNT
                ],
                ...
              ]
            ]
            //±AMOUNT	float	Total amount available at that price level. Trading: if AMOUNT > 0 then bid else ask; Funding: if AMOUNT < 0 then bid else ask;
            //[1436,[[6462.1,1,0.44900651],[6462,1,0.47744492],[6461.8,1,0.37530027],[6460.9,1,0.02333184],[6460.2,1,0.22609269],[6460,3,56.42399791],[6458,1,0.03622],[6457,1,1.54],[6456.6,1,0.4],[6456.1,2,0.8],[6455.5,1,2.313],[6454.9,1,0.4],[6453.6,1,0.2],[6452,2,1.2],[6451.8,2,0.6],[6451.7,1,1.5],[6450.1,1,3],[6450,1,0.5],[6449,1,0.15],[6448.7,2,0.00708406],[6448.6,3,1.53750584],[6448.5,2,64.00891964],[6448.3,1,0.028],[6448.2,1,1.55],[6447.8,1,7.98930678],[6462.2,28,-18.24119121],[6462.3,2,-0.26],[6462.4,1,-2.01620248],[6462.7,1,-0.19283279],[6462.8,1,-1.4],[6462.9,1,-1.5],[6463.1,1,-5.835],[6463.2,2,-1.373],[6463.4,1,-0.35391244],[6463.7,1,-0.30874569],[6463.8,1,-2.5],[6463.9,2,-2.54],[6464,1,-20],[6464.1,1,-0.5],[6464.2,1,-0.02217419],[6464.4,1,-0.31008094],[6464.6,1,-1],[6464.7,2,-0.22977],[6464.8,1,-1.85],[6465,1,-1],[6465.1,1,-2],[6465.7,2,-1.079],[6466.1,2,-1.544342],[6466.4,1,-1.3],[6467.2,1,-1.7]]]
             *
             *
            Algorithm to create and keep a book instance updated

            subscribe to channel
            receive the book snapshot and create your in-memory book structure
            when count > 0 then you have to add or update the price level
            3.1 if amount > 0 then add/update bids
            3.2 if amount < 0 then add/update asks
            when count = 0 then you have to delete the price level.
            4.1 if amount = 1 then remove from bids
            4.2 if amount = -1 then remove from asks
             * */
            for (int i = 0; i < len; i++) {
                int64_t price = std::round(json.GetArray()[last_element].GetArray()[i].GetArray()[0].GetDouble() * scale_offset);
                int count = json.GetArray()[last_element].GetArray()[i].GetArray()[1].GetInt();
                double dAmount = json.GetArray()[last_element].GetArray()[i].GetArray()[2].GetDouble();
                uint64_t amount = 0;
                if(dAmount < 0) {
                    amount = std::round(-1 * dAmount * scale_offset);
                } else {
                    amount = std::round( dAmount * scale_offset);
                }

                KF_LOG_INFO(logger, "MDEnginePoloniex::onBook: (ticker) " << ticker << " (price)" << price << " (amount)" << amount);

                if (count == 0) {
                    if(dAmount == 1 ) {
                        priceBook20Assembler.EraseBidPrice(ticker, price);
                    }
                    if(dAmount == -1 ) {
                        priceBook20Assembler.EraseAskPrice(ticker, price);
                    }
                } else if (count > 0) {
                    if(dAmount > 0) {
                        priceBook20Assembler.UpdateBidPrice(ticker, price, amount);
                    } else if(dAmount <= 0 ) {
                        priceBook20Assembler.UpdateAskPrice(ticker, price, amount);
                    }
                }
//                KF_LOG_INFO(logger, " (0)" << json.GetArray()[last_element].GetArray()[i].GetArray()[0].GetDouble() );
//                KF_LOG_INFO(logger, " (1)" << json.GetArray()[last_element].GetArray()[i].GetArray()[1].GetInt() );
//                KF_LOG_INFO(logger, " (2)" << json.GetArray()[last_element].GetArray()[i].GetArray()[2].GetDouble() );
            }
        } else {
            /*update
             * [
                  CHANNEL_ID,
                  [
                    PRICE,
                    COUNT,
                    AMOUNT
                  ]
                ]
            //±AMOUNT	float	Total amount available at that price level. Trading: if AMOUNT > 0 then bid else ask; Funding: if AMOUNT < 0 then bid else ask;
             //[1436,[6462.7,0,-1]]
             //[5,[6464.8,2,-1.90818689]]
             * */
            int64_t price = std::round(json.GetArray()[last_element].GetArray()[0].GetDouble() * scale_offset);
            int count = json.GetArray()[last_element].GetArray()[1].GetInt() ;
            double dAmount = json.GetArray()[last_element].GetArray()[2].GetDouble();
            uint64_t amount = 0;
            if(dAmount < 0) {
                amount = std::round(-1 * dAmount * scale_offset);
            } else {
                amount = std::round( dAmount * scale_offset);
            }

            KF_LOG_INFO(logger, "MDEnginePoloniex::onBook: (ticker) " << ticker << " (price)" << price << " (amount)" << amount);

            if (count == 0) {
                if(dAmount == 1 ) {
                    priceBook20Assembler.EraseBidPrice(ticker, price);
                }
                if(dAmount == -1 ) {
                    priceBook20Assembler.EraseAskPrice(ticker, price);
                }
            } else if (count > 0) {
                if(dAmount > 0) {
                    priceBook20Assembler.UpdateBidPrice(ticker, price, amount);
                } else if(dAmount <= 0 ) {
                    priceBook20Assembler.UpdateAskPrice(ticker, price, amount);
                }
            }
//            KF_LOG_INFO(logger, " update(0)" << json.GetArray()[last_element].GetArray()[0].GetDouble() );
//            KF_LOG_INFO(logger, " update(1)" << json.GetArray()[last_element].GetArray()[1].GetInt() );
//            KF_LOG_INFO(logger, " update(2)"<< json.GetArray()[last_element].GetArray()[2].GetDouble() );
        }
    }

    // has any update
    LFPriceBook20Field md;
    memset(&md, 0, sizeof(md));
    if(priceBook20Assembler.Assembler(ticker, md)) {
        strcpy(md.ExchangeID, "bitfinex");

        KF_LOG_INFO(logger, "MDEnginePoloniex::onDepth: on_price_book_update");
        on_price_book_update(&md);
    }
}

std::string MDEnginePoloniex::parseJsonToString(Document &d)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);

    return buffer.GetString();
}


//{ "command": "subscribe", "channel": "BTC_BTS" }
std::string MDEnginePoloniex::createBookJsonString(std::string exchange_coinpair)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("command");
    writer.String("subscribe");

    writer.Key("channel");
    writer.String(exchange_coinpair.c_str());

    writer.EndObject();
    return s.GetString();
}


void MDEnginePoloniex::loop()
{
    while(isRunning)
    {
        int n = lws_service( context, rest_get_interval_ms );
        std::cout << " 3.1415 loop() lws_service (n)" << n << std::endl;
    }
}

BOOST_PYTHON_MODULE(libbitfinexmd)
{
    using namespace boost::python;
    class_<MDEnginePoloniex, boost::shared_ptr<MDEnginePoloniex> >("Engine")
            .def(init<>())
            .def("init", &MDEnginePoloniex::initialize)
            .def("start", &MDEnginePoloniex::start)
            .def("stop", &MDEnginePoloniex::stop)
            .def("logout", &MDEnginePoloniex::logout)
            .def("wait_for_stop", &MDEnginePoloniex::wait_for_stop);
}
