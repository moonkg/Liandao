
#ifndef PROJECT_TDENGINEBITFINEX_H
#define PROJECT_TDENGINEBITFINEX_H

#include "ITDEngine.h"
#include "longfist/LFConstants.h"
#include "CoinPairWhiteList.h"
#include <vector>
#include <sstream>
#include <map>
#include <atomic>
#include <mutex>
#include "Timer.h"
#include <document.h>
#include <libwebsockets.h>


using rapidjson::Document;

WC_NAMESPACE_START

/**
 * account information unit extra is here.
 */
struct PositionSetting
{

    string ticker;
    bool isLong;
    uint64_t amount;

};

struct AccountUnitBitfinex
{
    string api_key;
    string secret_key;
    string baseUrl;
    // internal flags
    bool    logged_in;

    CoinPairWhiteList coinPairWhiteList;
    CoinPairWhiteList positionWhiteList;
    std::vector<std::string> newPendingSendMsg;
    std::vector<std::string> pendingSendMsg;
    struct lws * websocketConn;
};

struct OrderInsertData
{
    LFInputOrderField data;
    int requestId;
    //交易所反馈的orderId,在通知消息on_req能得到这个数据, findOrderRefByOrderid() will use this, for the trade info only has orderId
    int64_t remoteOrderId;

    //    撤单有两种，一种是用orderid 另一种是用clientid + date ,所以在发单时候记录一下当前日期，留待撤单时使用
    //            如果撤单时发现有orderid  会优先使用orderid ，没有orderid 的时候，才会使用clientid + date
    //            这种情况可能发生在 我们发单以后， on-req还没有来得及返回orderid，我们就发送撤单指令了
    std::string dateStr;
};

struct OrderActionData
{
    LFOrderActionField data;
    int requestId;
};

/**
 * CTP trade engine
 */
class TDEngineBitfinex: public ITDEngine
{
public:
    /** init internal journal writer (both raw and send) */
    virtual void init();
    /** for settleconfirm and authenticate setting */
    virtual void pre_load(const json& j_config);
    virtual TradeAccount load_account(int idx, const json& j_account);
    virtual void resize_accounts(int account_num);
    /** connect && login related */
    virtual void connect(long timeout_nsec);
    virtual void login(long timeout_nsec);
    virtual void logout();
    virtual void release_api();
    virtual bool is_connected() const;
    virtual bool is_logged_in() const;
    virtual string name() const { return "TDEngineBitfinex"; };

    // req functions
    virtual void req_investor_position(const LFQryPositionField* data, int account_index, int requestId);
    virtual void req_qry_account(const LFQryAccountField* data, int account_index, int requestId);
    virtual void req_order_insert(const LFInputOrderField* data, int account_index, int requestId, long rcv_time);
    virtual void req_order_action(const LFOrderActionField* data, int account_index, int requestId, long rcv_time);

public:
    TDEngineBitfinex();
    ~TDEngineBitfinex();


    void on_lws_data(struct lws* conn, const char* data, size_t len);
    void on_lws_connection_error(struct lws* conn);
    int lws_write_subscribe(struct lws* conn);
private:
    // journal writers
    yijinjing::JournalWriterPtr raw_writer;
    vector<AccountUnitBitfinex> account_units;


    std::string GetType(const LfOrderPriceTypeType& input);
    LfOrderPriceTypeType GetPriceType(std::string input);
    LfOrderStatusType GetOrderStatus(std::string input);

    virtual void set_reader_thread() override;
    void loop();

    inline int64_t getTimestamp();

    std::string createAuthJsonString(AccountUnitBitfinex& unit );
    std::string parseJsonToString(Document &d);
    std::string createInsertOrderJsonString(int gid, int cid, std::string type, std::string symbol, std::string amountStr, std::string priceStr);
    std::string createCancelOrderIdJsonString(int64_t orderId);
    std::string createCancelOrderCIdJsonString(int cid, std::string dateStr);
    std::string getDateStr();

    void lws_login(AccountUnitBitfinex& unit, long timeout_nsec);
    void onInfo(Document& json);
    void onAuth(struct lws * websocketConn, Document& json);
    void onPing(Document& json);
    void onPosition(struct lws * websocketConn, Document& json);
    void onTradeExecuted(struct lws * websocketConn, Document& json);
    void onTradeExecutionUpdate(struct lws * websocketConn, Document& json);
    void onOrderSnapshot(struct lws * websocketConn, Document& json);
    void onOrderNewUpdateCancel(struct lws * websocketConn, Document& json);
    void onNotification(struct lws * websocketConn, Document& json);


    void onOrder(struct lws * websocketConn, rapidjson::Value& json);


    AccountUnitBitfinex& findAccountUnitBitfinexByWebsocketConn(struct lws * websocketConn);

    void addPendingSendMsg(AccountUnitBitfinex& unit, std::string msg);
    void moveNewtoPending(AccountUnitBitfinex& unit);

    OrderInsertData findOrderInsertDataByOrderId(int64_t orderId);
    OrderInsertData findOrderInsertDataByOrderRef(const char_21 orderRef);


    bool startWith(const string &str, const string &head) {
        return str.compare(0, head.size(), head) == 0;
    }
private:

    static constexpr int scale_offset = 1e8;
    struct lws_context *context = nullptr;

    ThreadPtr rest_thread;

    std::vector<PositionSetting> positionHolder;

    uint64_t rest_get_interval_ms = 500;

    std::mutex* mutex_order_and_trade = nullptr;

    int64_t timeDiffOfExchange = 0;
    int exchange_shift_ms = 0;

    std::unordered_map<int, OrderInsertData> CIDorderInsertData;
    std::unordered_map<int, OrderActionData> CIDorderActionData;
    std::unordered_map<int64_t, OrderActionData> RemoteOrderIDorderActionData;
};

WC_NAMESPACE_END

#endif //PROJECT_TDENGINEBITFINEX_H


