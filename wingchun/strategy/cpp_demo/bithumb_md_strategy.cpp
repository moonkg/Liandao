/**
 * Strategy demo, same logic as band_demo_strategy.py
 * @Author cjiang (changhao.jiang@taurus.ai)
 * @since   Nov, 2017
 */

#include "IWCStrategy.h"
#include <deque>
#include <sstream>
USING_WC_NAMESPACE

#define SOURCE_INDEX SOURCE_BITHUMB
#define M_TICKER "btc"
#define M_EXCHANGE EXCHANGE_SHFE
#define TRADED_VOLUME_LIMIT 500


struct Signal
{
    string name;
    int look_back;
    int param1;
    int param2;
    int trade_size;
    std::deque<double> TickPrice;
    bool has_open_position;
    bool has_open_long_position;
    bool has_open_short_position;
};

class Strategy: public IWCStrategy
{
protected:
    bool td_connected;
    bool trade_completed;
    int rid;
    int md_num;
    int traded_volume;
    Signal signal;
public:
    virtual void init();
    virtual void on_market_data(const LFMarketDataField* data, short source, long rcv_time);
    virtual void on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time);
    //virtual void on_rtn_trade(const LFRtnTradeField* data, int request_id, short source, long rcv_time);
    //virtual void on_rsp_order(const LFInputOrderField* data, int request_id, short source, long rcv_time, short errorId=0, const char* errorMsg=nullptr);
    virtual void on_price_book_update(const LFPriceBook20Field* data, short source, long rcv_time);
    virtual void on_l2_trade(const LFL2TradeField* data, short source, long rcv_time);

public:
    Strategy(const string& name);
};

Strategy::Strategy(const string& name): IWCStrategy(name)
{
    std::cout << "[Strategy] (Strategy)" <<std::endl;
    rid = -1;

}

void Strategy::init()
{
    std::cout << "[Strategy] (init)" <<std::endl;
    KF_LOG_DEBUG(logger, "[init] SOURCE_INDEX:" << SOURCE_INDEX);
    data->add_market_data(SOURCE_INDEX);
    data->add_register_td(SOURCE_INDEX);
    vector<string> tickers;
    tickers.push_back(M_TICKER);
    util->subscribeMarketData(tickers, SOURCE_INDEX);
    // necessary initialization of internal fields.
    td_connected = false;
    trade_completed = true;
    md_num = 0;
    traded_volume = 0;
    // ========= bind and initialize a signal ========
    signal.name = "sample_signal";
    signal.look_back = 1000;
    signal.param1 = 200;
    signal.param2 = 50;
    signal.TickPrice.clear();
    signal.has_open_position = false;
    signal.has_open_long_position = false;
    signal.has_open_short_position = false;
    signal.trade_size = 1;
    int my_order_id = 0;
    std::cout << "[Strategy] (init) end" <<std::endl;
}

void Strategy::on_rsp_position(const PosHandlerPtr posMap, int request_id, short source, long rcv_time)
{
    if (request_id == -1 && source == SOURCE_INDEX)
    {
        td_connected = true;
        KF_LOG_INFO(logger, "td connected");
        if (posMap.get() == nullptr)
        {
            data->set_pos(PosHandlerPtr(new PosHandler(source)), source);
        }
    }
    else
    {
        KF_LOG_DEBUG(logger, "[RSP_POS] " << posMap->to_string());
    }
}

void Strategy::on_market_data(const LFMarketDataField* md, short source, long rcv_time)
{
    std::cout << "[on_market_data] (source)"<< source << " (InstrumentID)" << md->InstrumentID <<  "(AskPrice1)" << md->AskPrice1<<std::endl;
}

void Strategy::on_price_book_update(const LFPriceBook20Field* data, short source, long rcv_time)
{
    std::stringstream ss;
    ss << "LCK RECV[on_price_book_update] (source)" << source << " (ticker)" << data->InstrumentID << std::endl;
    for(int i = 0 ; i < data->BidLevelCount || i < data->AskLevelCount; ++i)
    {
        if(i < data->BidLevelCount)
        {
            ss << "BidPrice" << i+1 << "["<< data->BidLevels[i].price<<"]/t BidVolume" << i+1 << "["<< data->BidLevels[i].volume <<"]";
        }
        else
        {
            ss << "-- -- /t -- --";
        }
        ss << "/t";
        if( i < data->AskLevelCount)
        { 
            ss << "AskPrice" << i+1 << "[" << data->AskLevels[i].price <<"]/t AskVolume" << i+1 << "[" << data->AskLevels[i].volume << "]" << std::endl;
        }
        else
        {
             ss << "-- -- /t -- --" << std::endl;
        } 
    }
       
    std::cout << ss.str();
    KF_LOG_INFO(logger, ss.str());
}

void Strategy::on_l2_trade(const LFL2TradeField* data, short source, long rcv_time)
{
    std::stringstream ss ;
    ss << "LCK RECV[on_l2_trade] (source)" << source << " (ticker)" << data->InstrumentID << std::endl
	<< " Price[" << data->Price << "] /t Volume[" << data->Volume << "] /t OrderBSFlag["<< data->OrderBSFlag << "]" << std::endl;
    std::cout << ss.str();
    KF_LOG_INFO(logger, ss.str());
}

int main(int argc, const char* argv[])
{
    Strategy str(string("YOUR_STRATEGY2"));
    str.init();
    str.start();
    str.block();
    return 0;
}
