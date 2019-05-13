// auto generated by struct_info_parser.py, please DO NOT edit!!!

#ifndef LONGFIST_CONSTANTS_H
#define LONGFIST_CONSTANTS_H

#include <memory.h>
#include <cstdlib>
#include <stdio.h>
#include <string>

// Index for Sources...
enum exchange_source_index : short
{
    SOURCE_UNKNOWN =-1,
    SOURCE_CTP = 1,
    SOURCE_XTP = 15,
    SOURCE_BINANCE = 16,
    SOURCE_INDODAX = 17,
    SOURCE_OKEX = 18,
    SOURCE_COINMEX = 19,
    SOURCE_MOCK = 20,
    SOURCE_BITMAX = 21,
    SOURCE_BITFINEX = 22,
    SOURCE_BITTREX = 36,
    SOURCE_BITMEX = 23,
    SOURCE_HITBTC = 24,
    SOURCE_OCEANEX = 25,
    SOURCE_HUOBI = 26,
	SOURCE_OCEANEXB = 27,
    SOURCE_PROBIT = 28,
    SOURCE_BITHUMB = 29,
    SOURCE_DAYBIT = 31  
};

inline const char* get_str_from_source_index(exchange_source_index source)
{
    switch(source)
    {
        case SOURCE_CTP:
            return "ctp";
        case SOURCE_XTP:
            return "xtp";
        case SOURCE_BINANCE:
            return "binance";
        case SOURCE_INDODAX:
            return "indodax";
        case SOURCE_OKEX:
            return "okex";
        case SOURCE_COINMEX:
            return "coinmex";
        case SOURCE_MOCK:
            return "mock";
        case SOURCE_BITMAX:
            return "bitmax";
        case SOURCE_BITFINEX:
            return "bitfinex";
        case SOURCE_BITMEX:
            return "bitmex";
        case SOURCE_HITBTC:
            return "hitbtc";
        case SOURCE_OCEANEX:
            return "oceanex";
        case SOURCE_HUOBI:
            return "huobi";
        case SOURCE_OCEANEXB:
            return "oceanexb";
        case SOURCE_PROBIT:
            return "probit";
        case SOURCE_BITHUMB:
            return "bithumb";
        case SOURCE_DAYBIT:
            return "daybit";
        case SOURCE_BITTREX:
            return "bittrex";
		default:
			return "unknown";
	}
}

inline exchange_source_index get_source_index_from_str(const std::string& exch_str)
{
    if(exch_str == "ctp")
    {
        return SOURCE_CTP;
    }
    else if(exch_str == "xtp")
    {
        return SOURCE_XTP;
    }
    else if(exch_str == "binance")
    {
        return SOURCE_BINANCE;
    }
    else if(exch_str == "indodax")
    {
        return SOURCE_INDODAX;
    }
    else if(exch_str == "okex")
    {
        return SOURCE_OKEX;
    }
    else if(exch_str == "coinmex")
    {
        return SOURCE_COINMEX;
    }
    else if(exch_str == "mock")
    {
        return SOURCE_MOCK;
    }
    else if(exch_str == "bitmax")
    {
        return SOURCE_BITMAX;
    }
    else if(exch_str == "bitfinex")
    {
        return SOURCE_BITFINEX;
    }
    else if(exch_str == "bitmex")
    {
        return SOURCE_BITMEX;
    }
    else if(exch_str == "hitbtc")
    {
        return SOURCE_HITBTC;
    }
    else if(exch_str == "oceanex")
    {
        return SOURCE_OCEANEX;
    }
    else if(exch_str == "huobi")
    {
        return SOURCE_HUOBI;
    }
	else if (exch_str == "oceanexb")
	{
		return SOURCE_OCEANEXB;
	}
    else if(exch_str == "probit")
    {
        return SOURCE_PROBIT;
    }
    else if(exch_str == "daybit")
    {
        return SOURCE_DAYBIT;
    }
    else if(exch_str == "bithumb")
    {
		return SOURCE_BITHUMB;
    }
    else if(exch_str == "bittrex")
    {
        return SOURCE_BITTREX;
    }
    else 
    {
		return SOURCE_UNKNOWN;
    }

}

// Exchange names
#define EXCHANGE_SSE "SSE" //上海证券交易所
#define EXCHANGE_SZE "SZE" //深圳证券交易所
#define EXCHANGE_CFFEX "CFFEX" //中国金融期货交易所
#define EXCHANGE_SHFE "SHFE" //上海期货交易所
#define EXCHANGE_DCE "DCE" //大连商品交易所
#define EXCHANGE_CZCE "CZCE" //郑州商品交易所
#define EXCHANGE_BINANCE "BINANCE"
#define EXCHANGE_INDODAX "INDODAX"
#define EXCHANGE_OKEX "OKEX"
#define EXCHANGE_COINMEX "COINMEX"
#define EXCHANGE_MOCK "MOCK"
#define EXCHANGE_BITMAX "BITMAX"
#define EXCHANGE_BITFINEX "BITFINEX"
#define EXCHANGE_BITMEX "BITMEX"
#define EXCHANGE_HITBTC "HITBTC"
#define EXCHANGE_OCEANEX "OCEANEX"
#define EXCHANGE_HUOBI "HUOBI"
#define EXCHANGE_OCEANEXB "OCEANEXB"
#define EXCHANGE_PROBIT "PROBIT"
#define EXCHANGE_BITHUMB "BITHUMB"
#define EXCHANGE_DAYBIT "DAYBIT"
#define EXCHANGE_BITTREX "BITTREX"

// Exchange ids
#define EXCHANGE_ID_SSE 1 //上海证券交易所
#define EXCHANGE_ID_SZE 2 //深圳证券交易所
#define EXCHANGE_ID_CFFEX 11 //中国金融期货交易所
#define EXCHANGE_ID_SHFE 12 //上海期货交易所
#define EXCHANGE_ID_DCE 13 //大连商品交易所
#define EXCHANGE_ID_CZCE 14 //郑州商品交易所
#define EXCHANGE_ID_BINANCE  16
#define EXCHANGE_ID_INDODAX  17
#define EXCHANGE_ID_OKEX  18
#define EXCHANGE_ID_COINMEX  19
#define EXCHANGE_ID_MOCK  20
#define EXCHANGE_ID_BITMAX  21
#define EXCHANGE_ID_BITFINEX  22
#define EXCHANGE_ID_BITMEX  23
#define EXCHANGE_ID_HITBTC  24
#define EXCHANGE_ID_OCEANEX  25
#define EXCHANGE_ID_HUOBI  26
#define EXCHANGE_ID_OCEANEXB  27
#define EXCHANGE_ID_PROBIT  28
#define EXCHANGE_ID_BITHUMB  29
#define EXCHANGE_ID_DAYBIT  31
#define EXCHANGE_ID_BITTREX  36
// MsgTypes that used for LF data structure...
const short MSG_TYPE_LF_MD            = 101;
const short MSG_TYPE_LF_L2_MD         = 102;
const short MSG_TYPE_LF_L2_INDEX      = 103;
const short MSG_TYPE_LF_L2_ORDER      = 104;
const short MSG_TYPE_LF_L2_TRADE      = 105;
const short MSG_TYPE_LF_PRICE_BOOK_20 = 106;
const short MSG_TYPE_LF_BAR_MD        = 110;
const short MSG_TYPE_LF_FUNDING       = 111;
const short MSG_TYPE_LF_QRY_POS       = 201;
const short MSG_TYPE_LF_RSP_POS       = 202;
const short MSG_TYPE_LF_ORDER         = 204;
const short MSG_TYPE_LF_RTN_ORDER     = 205;
const short MSG_TYPE_LF_RTN_TRADE     = 206;
const short MSG_TYPE_LF_ORDER_ACTION  = 207;
const short MSG_TYPE_LF_QRY_ACCOUNT   = 208;
const short MSG_TYPE_LF_RSP_ACCOUNT   = 209;

// MsgTypes that from original data structures...
// ctp, idx=1
const short MSG_TYPE_LF_MD_CTP        = 1101; // CThostFtdcDepthMarketDataField from ctp/ThostFtdcUserApiStruct.h
const short MSG_TYPE_LF_QRY_POS_CTP   = 1201; // CThostFtdcQryInvestorPositionField from ctp/ThostFtdcUserApiStruct.h
const short MSG_TYPE_LF_RSP_POS_CTP   = 1202; // CThostFtdcInvestorPositionField from ctp/ThostFtdcUserApiStruct.h
const short MSG_TYPE_LF_ORDER_CTP     = 1204; // CThostFtdcInputOrderField from ctp/ThostFtdcUserApiStruct.h
const short MSG_TYPE_LF_RTN_ORDER_CTP = 1205; // CThostFtdcOrderField from ctp/ThostFtdcUserApiStruct.h
const short MSG_TYPE_LF_RTN_TRADE_CTP = 1206; // CThostFtdcTradeField from ctp/ThostFtdcUserApiStruct.h
const short MSG_TYPE_LF_ORDER_ACTION_CTP = 1207; // CThostFtdcInputOrderActionField from ctp/ThostFtdcUserApiStruct.h
const short MSG_TYPE_LF_QRY_ACCOUNT_CTP = 1208; // CThostFtdcQryTradingAccountField from ctp/ThostFtdcUserApiStruct.h
const short MSG_TYPE_LF_RSP_ACCOUNT_CTP = 1209; // CThostFtdcTradingAccountField from ctp/ThostFtdcUserApiStruct.h

// xtp, idx=15
const short MSG_TYPE_LF_MD_XTP        = 15101; // XTPMarketDataStruct from xtp/xquote_api_struct.h
const short MSG_TYPE_LF_RSP_POS_XTP   = 15202; // XTPQueryStkPositionRsp from xtp/xoms_api_struct.h
const short MSG_TYPE_LF_ORDER_XTP     = 15204; // XTPOrderInsertInfo from xtp/xoms_api_struct.h
const short MSG_TYPE_LF_RTN_ORDER_XTP = 15205; // XTPOrderInfo from xtp/xoms_api_struct.h
const short MSG_TYPE_LF_RTN_TRADE_XTP = 15206; // XTPTradeReport from xtp/xoms_api_struct.h

//binance, idx=16
const short MSG_TYPE_LF_MD_BINANCE        = 16101;
const short MSG_TYPE_LF_QRY_POS_BINANCE   = 16201;
const short MSG_TYPE_LF_RSP_POS_BINANCE   = 16202;
const short MSG_TYPE_LF_ORDER_BINANCE     = 16204; 
const short MSG_TYPE_LF_RTN_ORDER_BINANCE = 16205; 
const short MSG_TYPE_LF_RTN_TRADE_BINANCE = 16206; 
const short MSG_TYPE_LF_ORDER_ACTION_BINANCE = 16207; 

//indodax, idx=17
const short MSG_TYPE_LF_MD_INDODAX        = 17101; 
const short MSG_TYPE_LF_ORDER_INDODAX    = 17204; 
const short MSG_TYPE_LF_RTN_ORDER_INDODAX = 17205; 
const short MSG_TYPE_LF_RTN_TRADE_INDODAX = 17206; 
const short MSG_TYPE_LF_ORDER_ACTION_INDODAX = 17207; 

//okex, idx=18
const short MSG_TYPE_LF_MD_OKEX        = 18101; 
const short MSG_TYPE_LF_ORDER_OKEX    = 18204; 
const short MSG_TYPE_LF_RTN_ORDER_OKEX = 18205; 
const short MSG_TYPE_LF_RTN_TRADE_OKEX = 18206; 
const short MSG_TYPE_LF_ORDER_ACTION_OKEX = 18207;

//coinmex, idx=19
const short MSG_TYPE_LF_MD_COINMEX        = 19101;
const short MSG_TYPE_LF_QRY_POS_COINMEX   = 19201;
const short MSG_TYPE_LF_RSP_POS_COINMEX   = 19202;
const short MSG_TYPE_LF_ORDER_COINMEX     = 19204;
const short MSG_TYPE_LF_RTN_ORDER_COINMEX = 19205;
const short MSG_TYPE_LF_RTN_TRADE_COINMEX = 19206;
const short MSG_TYPE_LF_ORDER_ACTION_COINMEX = 19207;

//mock, idx=20
const short MSG_TYPE_LF_MD_MOCK        = 20101;
const short MSG_TYPE_LF_QRY_POS_MOCK   = 20201;
const short MSG_TYPE_LF_RSP_POS_MOCK   = 20202;
const short MSG_TYPE_LF_ORDER_MOCK     = 20204;
const short MSG_TYPE_LF_RTN_ORDER_MOCK = 20205;
const short MSG_TYPE_LF_RTN_TRADE_MOCK = 20206;
const short MSG_TYPE_LF_ORDER_ACTION_MOCK = 20207;

//bitmax, idx=21
const short MSG_TYPE_LF_MD_BITMAX        = 21101;
const short MSG_TYPE_LF_QRY_POS_BITMAX   = 21201;
const short MSG_TYPE_LF_RSP_POS_BITMAX   = 21202;
const short MSG_TYPE_LF_ORDER_BITMAX     = 21204;
const short MSG_TYPE_LF_RTN_ORDER_BITMAX = 21205;
const short MSG_TYPE_LF_RTN_TRADE_BITMAX = 21206;
const short MSG_TYPE_LF_ORDER_ACTION_BITMAX = 21207;

//bitfinex, idx=22
const short MSG_TYPE_LF_MD_BITFINEX        = 22101;
const short MSG_TYPE_LF_QRY_POS_BITFINEX   = 22201;
const short MSG_TYPE_LF_RSP_POS_BITFINEX   = 22202;
const short MSG_TYPE_LF_ORDER_BITFINEX     = 22204;
const short MSG_TYPE_LF_RTN_ORDER_BITFINEX = 22205;
const short MSG_TYPE_LF_RTN_TRADE_BITFINEX = 22206;
const short MSG_TYPE_LF_ORDER_ACTION_BITFINEX = 22207;

//bitmex, idx=23
const short MSG_TYPE_LF_MD_BITMEX        = 23101;
const short MSG_TYPE_LF_QRY_POS_BITMEX   = 23201;
const short MSG_TYPE_LF_RSP_POS_BITMEX   = 23202;
const short MSG_TYPE_LF_ORDER_BITMEX     = 23204;
const short MSG_TYPE_LF_RTN_ORDER_BITMEX = 23205;
const short MSG_TYPE_LF_RTN_TRADE_BITMEX = 23206;
const short MSG_TYPE_LF_ORDER_ACTION_BITMEX = 23207;

//HITBTC, idx=24
const short MSG_TYPE_LF_MD_HITBTC        = 24101;
const short MSG_TYPE_LF_QRY_POS_HITBTC   = 24201;
const short MSG_TYPE_LF_RSP_POS_HITBTC   = 24202;
const short MSG_TYPE_LF_ORDER_HITBTC     = 24204;
const short MSG_TYPE_LF_RTN_ORDER_HITBTC = 24205;
const short MSG_TYPE_LF_RTN_TRADE_HITBTC = 24206;
const short MSG_TYPE_LF_ORDER_ACTION_HITBTC = 24207;

//OCEANEX, idx=25
const short MSG_TYPE_LF_MD_OCEANEX        = 25101;
const short MSG_TYPE_LF_QRY_POS_OCEANEX   = 25201;
const short MSG_TYPE_LF_RSP_POS_OCEANEX   = 25202;
const short MSG_TYPE_LF_ORDER_OCEANEX     = 25204;
const short MSG_TYPE_LF_RTN_ORDER_OCEANEX = 25205;
const short MSG_TYPE_LF_RTN_TRADE_OCEANEX = 25206;
const short MSG_TYPE_LF_ORDER_ACTION_OCEANEX = 25207;

//HUOBI, idx=26
const short MSG_TYPE_LF_MD_HUOBI = 26101;
const short MSG_TYPE_LF_QRY_POS_HUOBI = 26201;
const short MSG_TYPE_LF_RSP_POS_HUOBI = 26202;
const short MSG_TYPE_LF_ORDER_HUOBI = 26204;
const short MSG_TYPE_LF_RTN_ORDER_HUOBI = 26205;
const short MSG_TYPE_LF_RTN_TRADE_HUOBI = 26206;
const short MSG_TYPE_LF_ORDER_ACTION_HUOBI = 26207;

//OCEANEXB, idx=27
const short MSG_TYPE_LF_MD_OCEANEXB        = 27101;
const short MSG_TYPE_LF_QRY_POS_OCEANEXB   = 27201;
const short MSG_TYPE_LF_RSP_POS_OCEANEXB   = 27202;
const short MSG_TYPE_LF_ORDER_OCEANEXB     = 27204;
const short MSG_TYPE_LF_RTN_ORDER_OCEANEXB = 27205;
const short MSG_TYPE_LF_RTN_TRADE_OCEANEXB = 27206;
const short MSG_TYPE_LF_ORDER_ACTION_OCEANEXB = 27207;
//PROBIT, idx=28
const short MSG_TYPE_LF_MD_PROBIT        	= 28101;
const short MSG_TYPE_LF_QRY_POS_PROBIT   	= 28201;
const short MSG_TYPE_LF_RSP_POS_PROBIT   	= 28202;
const short MSG_TYPE_LF_ORDER_PROBIT     	= 28204;
const short MSG_TYPE_LF_RTN_ORDER_PROBIT 	= 28205;
const short MSG_TYPE_LF_RTN_TRADE_PROBIT 	= 28206;
const short MSG_TYPE_LF_ORDER_ACTION_PROBIT = 28207;

//BITHUMB, idx=29
const short MSG_TYPE_LF_MD_BITHUMB             = 29101;
const short MSG_TYPE_LF_QRY_POS_BITHUMB           = 29201;
const short MSG_TYPE_LF_RSP_POS_BITHUMB           = 29202;
const short MSG_TYPE_LF_ORDER_BITHUMB             = 29204;
const short MSG_TYPE_LF_RTN_ORDER_BITHUMB         = 29205;
const short MSG_TYPE_LF_RTN_TRADE_BITHUMB         = 29206;
const short MSG_TYPE_LF_ORDER_ACTION_BITHUMB     = 29207;



//DAYBIT, idx=31
const short MSG_TYPE_LF_MD_DAYBIT        	= 31101;
const short MSG_TYPE_LF_QRY_POS_DAYBIT   	= 31201;
const short MSG_TYPE_LF_RSP_POS_DAYBIT   	= 31202;
const short MSG_TYPE_LF_ORDER_DAYBIT     	= 31204;
const short MSG_TYPE_LF_RTN_ORDER_DAYBIT 	= 31205;
const short MSG_TYPE_LF_RTN_TRADE_DAYBIT 	= 31206;
const short MSG_TYPE_LF_ORDER_ACTION_DAYBIT = 31207;

//bittrex, idx=36
const short MSG_TYPE_LF_MD_BITTREX        = 36101;
const short MSG_TYPE_LF_QRY_POS_BITTREX   = 36201;
const short MSG_TYPE_LF_RSP_POS_BITTREX   = 36202;
const short MSG_TYPE_LF_ORDER_BITTREX     = 36204;
const short MSG_TYPE_LF_RTN_ORDER_BITTREX = 36205;
const short MSG_TYPE_LF_RTN_TRADE_BITTREX = 36206;
const short MSG_TYPE_LF_ORDER_ACTION_BITTREX = 36207;
///////////////////////////////////
// LfActionFlagType: 报单操作标志
///////////////////////////////////
//删除
#define LF_CHAR_Delete          '0'
//挂起
#define LF_CHAR_Suspend         '1'
//激活
#define LF_CHAR_Active          '2'
//修改
#define LF_CHAR_Modify          '3'

typedef char LfActionFlagType;

///////////////////////////////////
// LfContingentConditionType: 触发条件
///////////////////////////////////
//立即
#define LF_CHAR_Immediately     '1'
//止损
#define LF_CHAR_Touch           '2'
//止赢
#define LF_CHAR_TouchProfit     '3'
//预埋单
#define LF_CHAR_ParkedOrder     '4'
//最新价大于条件价
#define LF_CHAR_LastPriceGreaterThanStopPrice '5'
//最新价大于等于条件价
#define LF_CHAR_LastPriceGreaterEqualStopPrice '6'
//最新价小于条件价
#define LF_CHAR_LastPriceLesserThanStopPrice '7'
//最新价小于等于条件价
#define LF_CHAR_LastPriceLesserEqualStopPrice '8'
//卖一价大于条件价
#define LF_CHAR_AskPriceGreaterThanStopPrice '9'
//卖一价大于等于条件价
#define LF_CHAR_AskPriceGreaterEqualStopPrice 'A'
//卖一价小于条件价
#define LF_CHAR_AskPriceLesserThanStopPrice 'B'
//卖一价小于等于条件价
#define LF_CHAR_AskPriceLesserEqualStopPrice 'C'
//买一价大于条件价
#define LF_CHAR_BidPriceGreaterThanStopPrice 'D'
//买一价大于等于条件价
#define LF_CHAR_BidPriceGreaterEqualStopPrice 'E'
//买一价小于条件价
#define LF_CHAR_BidPriceLesserThanStopPrice 'F'
//买一价小于等于条件价
#define LF_CHAR_BidPriceLesserEqualStopPrice 'H'

typedef char LfContingentConditionType;

///////////////////////////////////
// LfDirectionType: 买卖方向
///////////////////////////////////
//买
#define LF_CHAR_Buy             '0'
//卖
#define LF_CHAR_Sell            '1'

typedef char LfDirectionType;

///////////////////////////////////
// LfForceCloseReasonType: 强平原因
///////////////////////////////////
//非强平
#define LF_CHAR_NotForceClose   '0'
//资金不足
#define LF_CHAR_LackDeposit     '1'
//客户超仓
#define LF_CHAR_ClientOverPositionLimit '2'
//会员超仓
#define LF_CHAR_MemberOverPositionLimit '3'
//持仓非整数倍
#define LF_CHAR_NotMultiple     '4'
//违规
#define LF_CHAR_Violation       '5'
//其它
#define LF_CHAR_Other           '6'
//自然人临近交割
#define LF_CHAR_PersonDeliv     '7'

typedef char LfForceCloseReasonType;

///////////////////////////////////
// LfHedgeFlagType: 投机套保标志
///////////////////////////////////
//投机
#define LF_CHAR_Speculation     '1'
//套利
#define LF_CHAR_Argitrage       '2'
//套保
#define LF_CHAR_Hedge           '3'
//做市商(femas)
#define LF_CHAR_MarketMaker     '4'
//匹配所有的值(femas)
#define LF_CHAR_AllValue        '9'

typedef char LfHedgeFlagType;

///////////////////////////////////
// LfOffsetFlagType: 开平标志
///////////////////////////////////
//开仓
#define LF_CHAR_Open            '0'
//平仓
#define LF_CHAR_Close           '1'
//强平
#define LF_CHAR_ForceClose      '2'
//平今
#define LF_CHAR_CloseToday      '3'
//平昨
#define LF_CHAR_CloseYesterday  '4'
//强减
#define LF_CHAR_ForceOff        '5'
//本地强平
#define LF_CHAR_LocalForceClose '6'
//不分开平
#define LF_CHAR_Non             'N'

typedef char LfOffsetFlagType;

///////////////////////////////////
// LfOrderPriceTypeType: 报单价格条件
///////////////////////////////////
//任意价
#define LF_CHAR_AnyPrice        '1'
//限价
#define LF_CHAR_LimitPrice      '2'
//最优价
#define LF_CHAR_BestPrice       '3'

typedef char LfOrderPriceTypeType;

///////////////////////////////////
// LfOrderStatusType: 报单状态
///////////////////////////////////
//全部成交（最终状态）
#define LF_CHAR_AllTraded       '0'
//部分成交还在队列中
#define LF_CHAR_PartTradedQueueing '1'
//部分成交不在队列中（部成部撤， 最终状态）
#define LF_CHAR_PartTradedNotQueueing '2'
//未成交还在队列中
#define LF_CHAR_NoTradeQueueing '3'
//未成交不在队列中（被拒绝，最终状态）
#define LF_CHAR_NoTradeNotQueueing '4'
//撤单
#define LF_CHAR_Canceled        '5'
//订单已报入交易所未应答
#define LF_CHAR_AcceptedNoReply '6'
//未知
#define LF_CHAR_Unknown         'a'
//尚未触发
#define LF_CHAR_NotTouched      'b'
//已触发
#define LF_CHAR_Touched         'c'
//废单错误（最终状态）
#define LF_CHAR_Error           'd'
//订单已写入
#define LF_CHAR_OrderInserted   'i'
//前置已接受
#define LF_CHAR_OrderAccepted   'j'

#define LF_CHAR_PendingCancel   'k'

typedef char LfOrderStatusType;

///////////////////////////////////
// LfPosiDirectionType: 持仓多空方向
///////////////////////////////////
//净
#define LF_CHAR_Net             '1'
//多头
#define LF_CHAR_Long            '2'
//空头
#define LF_CHAR_Short           '3'

typedef char LfPosiDirectionType;

///////////////////////////////////
// LfPositionDateType: 持仓日期
///////////////////////////////////
//今日持仓
#define LF_CHAR_Today           '1'
//历史持仓
#define LF_CHAR_History         '2'
//两种持仓
#define LF_CHAR_Both            '3'

typedef char LfPositionDateType;

///////////////////////////////////
// LfTimeConditionType: 有效期类型
///////////////////////////////////
//立即完成，否则撤销
#define LF_CHAR_IOC             '1'
//本节有效
#define LF_CHAR_GFS             '2'
//当日有效
#define LF_CHAR_GFD             '3'
//指定日期前有效
#define LF_CHAR_GTD             '4'
//撤销前有效
#define LF_CHAR_GTC             '5'
//集合竞价有效
#define LF_CHAR_GFA             '6'
//FAK或IOC(yisheng)
#define LF_CHAR_FAK             'A'
//FOK(yisheng)
#define LF_CHAR_FOK             'O'

typedef char LfTimeConditionType;

///////////////////////////////////
// LfVolumeConditionType: 成交量类型
///////////////////////////////////
//任何数量
#define LF_CHAR_AV              '1'
//最小数量
#define LF_CHAR_MV              '2'
//全部数量
#define LF_CHAR_CV              '3'

typedef char LfVolumeConditionType;

///////////////////////////////////
// LfYsHedgeFlagType: 易盛投机保值类型
///////////////////////////////////
//保值
#define LF_CHAR_YsB             'B'
//套利
#define LF_CHAR_YsL             'L'
//无
#define LF_CHAR_YsNon           'N'
//投机
#define LF_CHAR_YsT             'T'

typedef char LfYsHedgeFlagType;

///////////////////////////////////
// LfYsOrderStateType: 易盛委托状态类型
///////////////////////////////////
//终端提交
#define LF_CHAR_YsSubmit        '0'
//已受理
#define LF_CHAR_YsAccept        '1'
//策略待触发
#define LF_CHAR_YsTriggering    '2'
//交易所待触发
#define LF_CHAR_YsExctriggering '3'
//已排队
#define LF_CHAR_YsQueued        '4'
//部分成交
#define LF_CHAR_YsPartFinished  '5'
//完全成交
#define LF_CHAR_YsFinished      '6'
//待撤消(排队临时状态)
#define LF_CHAR_YsCanceling     '7'
//待修改(排队临时状态)
#define LF_CHAR_YsModifying     '8'
//完全撤单
#define LF_CHAR_YsCanceled      '9'
//已撤余单
#define LF_CHAR_YsLeftDeleted   'A'
//指令失败
#define LF_CHAR_YsFail          'B'
//策略删除
#define LF_CHAR_YsDeleted       'C'
//已挂起
#define LF_CHAR_YsSuppended     'D'
//到期删除
#define LF_CHAR_YsDeletedForExpire 'E'
//已生效——询价成功
#define LF_CHAR_YsEffect        'F'
//已申请——行权、弃权、套利等申请成功
#define LF_CHAR_YsApply         'G'

typedef char LfYsOrderStateType;

///////////////////////////////////
// LfYsOrderTypeType: 易盛委托类型
///////////////////////////////////
//市价
#define LF_CHAR_YsMarket        '1'
//限价
#define LF_CHAR_YsLimit         '2'

typedef char LfYsOrderTypeType;

///////////////////////////////////
// LfYsPositionEffectType: 易盛开平类型
///////////////////////////////////
//平仓
#define LF_CHAR_YsClose         'C'
//不分开平
#define LF_CHAR_YsNon           'N'
//开仓
#define LF_CHAR_YsOpen          'O'
//平当日
#define LF_CHAR_YsCloseToday    'T'

typedef char LfYsPositionEffectType;

///////////////////////////////////
// LfYsSideTypeType: 易盛买卖类型
///////////////////////////////////
//双边
#define LF_CHAR_YsAll           'A'
//买入
#define LF_CHAR_YsBuy           'B'
//无
#define LF_CHAR_YsNon           'N'
//卖出
#define LF_CHAR_YsSell          'S'

typedef char LfYsSideTypeType;

///////////////////////////////////
// LfYsTimeConditionType: 易盛委托有效类型
///////////////////////////////////
//当日有效
#define LF_CHAR_YsGFD           '0'
//撤销前有效
#define LF_CHAR_YsGTC           '1'
//指定日期前有效
#define LF_CHAR_YsGTD           '2'
//FAK或IOC
#define LF_CHAR_YsFAK           '3'
//FOK
#define LF_CHAR_YsFOK           '4'

typedef char LfYsTimeConditionType;

#endif
