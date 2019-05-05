// auto generated by struct_info_parser.py, please DO NOT edit!!!

#ifndef LONGFIST_DATA_STRUCTS_H
#define LONGFIST_DATA_STRUCTS_H

#include "LFConstants.h"

typedef char char_19[19];
typedef char char_21[21];
typedef char char_64[64];
typedef char char_7[7];
typedef char char_9[9];
typedef char char_30[30];
typedef char char_31[31];
typedef char char_16[16];
typedef char char_13[13];
typedef char char_2[2];
typedef char char_11[11];

struct LFMarketDataField
{
	char_13  	TradingDay;            //交易日
	char_31  	InstrumentID;          //合约代码
	char_9   	ExchangeID;            //交易所代码
	char_64  	ExchangeInstID;        //合约在交易所的代码
	double   	LastPrice;             //最新价
	double   	PreSettlementPrice;    //上次结算价
	double   	PreClosePrice;         //昨收盘
	double   	PreOpenInterest;       //昨持仓量
	double   	OpenPrice;             //今开盘
	double   	HighestPrice;          //最高价
	double   	LowestPrice;           //最低价
	int      	Volume;                //数量
	double   	Turnover;              //成交金额
	double   	OpenInterest;          //持仓量
	double   	ClosePrice;            //今收盘
	double   	SettlementPrice;       //本次结算价
	double   	UpperLimitPrice;       //涨停板价
	double   	LowerLimitPrice;       //跌停板价
	double   	PreDelta;              //昨虚实度
	double   	CurrDelta;             //今虚实度
	char_13  	UpdateTime;            //最后修改时间
	uint64_t      	UpdateMillisec;        //最后修改毫秒
	int64_t   	BidPrice1;             //申买价一
	uint64_t      	BidVolume1;            //申买量一
	int64_t   	AskPrice1;             //申卖价一
	uint64_t      	AskVolume1;            //申卖量一
	int64_t   	BidPrice2;             //申买价二
	uint64_t      	BidVolume2;            //申买量二
	int64_t   	AskPrice2;             //申卖价二
	uint64_t      	AskVolume2;            //申卖量二
	int64_t   	BidPrice3;             //申买价三
	uint64_t      	BidVolume3;            //申买量三
	int64_t   	AskPrice3;             //申卖价三
	uint64_t      	AskVolume3;            //申卖量三
	int64_t   	BidPrice4;             //申买价四
	uint64_t      	BidVolume4;            //申买量四
	int64_t   	AskPrice4;             //申卖价四
	uint64_t      	AskVolume4;            //申卖量四
	int64_t   	BidPrice5;             //申买价五
	uint64_t      	BidVolume5;            //申买量五
	int64_t   	AskPrice5;             //申卖价五
	uint64_t      	AskVolume5;            //申卖量五
};

struct LFPriceLevel
{
	int64_t price;
	uint64_t volume;
};

template<int level_count>
struct LFPriceBookFieldTmpl
{
	char_31  	InstrumentID;          
	char_9   	ExchangeID;           
	uint64_t      	UpdateMicroSecond;    
	int		BidLevelCount;	
	int		AskLevelCount;	
	LFPriceLevel 	BidLevels[level_count];
	LFPriceLevel	AskLevels[level_count];
    int Status;/*FXW's edits,0=normal 1=loss level 2=crossed*/
};

using LFPriceBook20Field = LFPriceBookFieldTmpl<20>;

struct LFL2MarketDataField
{
	char_9   	TradingDay;            //交易日
	char_9   	TimeStamp;             //时间戳
	char_9   	ExchangeID;            //交易所代码
	char_31  	InstrumentID;          //合约代码
	double   	PreClosePrice;         //昨收盘价
	double   	OpenPrice;             //今开盘价
	double   	ClosePrice;            //收盘价
	double   	IOPV;                  //净值估值
	double   	YieldToMaturity;       //到期收益率
	double   	AuctionPrice;          //动态参考价格
	char     	TradingPhase;          //交易阶段
	char     	OpenRestriction;       //开仓限制
	double   	HighPrice;             //最高价
	double   	LowPrice;              //最低价
	double   	LastPrice;             //最新价
	double   	TradeCount;            //成交笔数
	double   	TotalTradeVolume;      //成交总量
	double   	TotalTradeValue;       //成交总金额
	double   	OpenInterest;          //持仓量
	double   	TotalBidVolume;        //委托买入总量
	double   	WeightedAvgBidPrice;   //加权平均委买价
	double   	AltWeightedAvgBidPrice;   //债券加权平均委买价
	double   	TotalOfferVolume;      //委托卖出总量
	double   	WeightedAvgOfferPrice;   //加权平均委卖价
	double   	AltWeightedAvgOfferPrice;   //债券加权平均委卖价格
	int      	BidPriceLevel;         //买价深度
	int      	OfferPriceLevel;       //卖价深度
	double   	BidPrice1;             //申买价一
	double   	BidVolume1;            //申买量一
	int      	BidCount1;             //实际买总委托笔数一
	double   	BidPrice2;             //申买价二
	double   	BidVolume2;            //申买量二
	int      	BidCount2;             //实际买总委托笔数二
	double   	BidPrice3;             //申买价三
	double   	BidVolume3;            //申买量三
	int      	BidCount3;             //实际买总委托笔数三
	double   	BidPrice4;             //申买价四
	double   	BidVolume4;            //申买量四
	int      	BidCount4;             //实际买总委托笔数四
	double   	BidPrice5;             //申买价五
	double   	BidVolume5;            //申买量五
	int      	BidCount5;             //实际买总委托笔数五
	double   	BidPrice6;             //申买价六
	double   	BidVolume6;            //申买量六
	int      	BidCount6;             //实际买总委托笔数六
	double   	BidPrice7;             //申买价七
	double   	BidVolume7;            //申买量七
	int      	BidCount7;             //实际买总委托笔数七
	double   	BidPrice8;             //申买价八
	double   	BidVolume8;            //申买量八
	int      	BidCount8;             //实际买总委托笔数八
	double   	BidPrice9;             //申买价九
	double   	BidVolume9;            //申买量九
	int      	BidCount9;             //实际买总委托笔数九
	double   	BidPriceA;             //申买价十
	double   	BidVolumeA;            //申买量十
	int      	BidCountA;             //实际买总委托笔数十
	double   	OfferPrice1;           //申卖价一
	double   	OfferVolume1;          //申卖量一
	int      	OfferCount1;           //实际卖总委托笔数一
	double   	OfferPrice2;           //申卖价二
	double   	OfferVolume2;          //申卖量二
	int      	OfferCount2;           //实际卖总委托笔数二
	double   	OfferPrice3;           //申卖价三
	double   	OfferVolume3;          //申卖量三
	int      	OfferCount3;           //实际卖总委托笔数三
	double   	OfferPrice4;           //申卖价四
	double   	OfferVolume4;          //申卖量四
	int      	OfferCount4;           //实际卖总委托笔数四
	double   	OfferPrice5;           //申卖价五
	double   	OfferVolume5;          //申卖量五
	int      	OfferCount5;           //实际卖总委托笔数五
	double   	OfferPrice6;           //申卖价六
	double   	OfferVolume6;          //申卖量六
	int      	OfferCount6;           //实际卖总委托笔数六
	double   	OfferPrice7;           //申卖价七
	double   	OfferVolume7;          //申卖量七
	int      	OfferCount7;           //实际卖总委托笔数七
	double   	OfferPrice8;           //申卖价八
	double   	OfferVolume8;          //申卖量八
	int      	OfferCount8;           //实际卖总委托笔数八
	double   	OfferPrice9;           //申卖价九
	double   	OfferVolume9;          //申卖量九
	int      	OfferCount9;           //实际卖总委托笔数九
	double   	OfferPriceA;           //申卖价十
	double   	OfferVolumeA;          //申卖量十
	int      	OfferCountA;           //实际卖总委托笔数十
	char_7   	InstrumentStatus;      //合约状态
	double   	PreIOPV;               //昨净值估值
	double   	PERatio1;              //市盈率一
	double   	PERatio2;              //市盈率二
	double   	UpperLimitPrice;       //涨停价
	double   	LowerLimitPrice;       //跌停价
	double   	WarrantPremiumRatio;   //权证溢价率
	double   	TotalWarrantExecQty;   //权证执行总数量
	double   	PriceDiff1;            //升跌一
	double   	PriceDiff2;            //升跌二
	double   	ETFBuyNumber;          //ETF申购笔数
	double   	ETFBuyAmount;          //ETF申购数量
	double   	ETFBuyMoney;           //ETF申购金额
	double   	ETFSellNumber;         //ETF赎回笔数
	double   	ETFSellAmount;         //ETF赎回数量
	double   	ETFSellMoney;          //ETF赎回金额
	double   	WithdrawBuyNumber;     //买入撤单笔数
	double   	WithdrawBuyAmount;     //买入撤单数量
	double   	WithdrawBuyMoney;      //买入撤单金额
	double   	TotalBidNumber;        //买入总笔数
	double   	BidTradeMaxDuration;   //买入委托成交最大等待时间
	double   	NumBidOrders;          //买方委托价位数
	double   	WithdrawSellNumber;    //卖出撤单笔数
	double   	WithdrawSellAmount;    //卖出撤单数量
	double   	WithdrawSellMoney;     //卖出撤单金额
	double   	TotalOfferNumber;      //卖出总笔数
	double   	OfferTradeMaxDuration;   //卖出委托成交最大等待时间
	double   	NumOfferOrders;        //卖方委托价位数
};

struct LFFundingField
{
	char_31  	InstrumentID;
	char_9   	ExchangeID;
	int64_t     TimeStamp;
	int64_t		Interval;
	double		Rate;
	double 		RateDaily;
};
struct LFL2IndexField
{
	char_9   	TradingDay;            //交易日
	char_9   	TimeStamp;             //行情时间（秒）
	char_9   	ExchangeID;            //交易所代码
	char_31  	InstrumentID;          //指数代码
	double   	PreCloseIndex;         //前收盘指数
	double   	OpenIndex;             //今开盘指数
	double   	CloseIndex;            //今日收盘指数
	double   	HighIndex;             //最高指数
	double   	LowIndex;              //最低指数
	double   	LastIndex;             //最新指数
	double   	TurnOver;              //参与计算相应指数的成交金额（元）
	double   	TotalVolume;           //参与计算相应指数的交易数量（手）
};

struct LFL2OrderField
{
	char_9   	OrderTime;             //委托时间（秒）
	char_9   	ExchangeID;            //交易所代码
	char_31  	InstrumentID;          //合约代码
	double   	Price;                 //委托价格
	double   	Volume;                //委托数量
	char_2   	OrderKind;             //报单类型
};

struct LFL2TradeField
{
	char_9   	TradeTime;             //成交时间（秒）
	char_9   	ExchangeID;            //交易所代码
	char_31  	InstrumentID;          //合约代码
	int64_t   	Price;                 //成交价格
	uint64_t   	Volume;                //成交数量
	char_2   	OrderKind;             //报单类型
	char_2   	OrderBSFlag;           //内外盘标志
    int         Status;                 //状态码 0为正常//quest3 edited by fxw
};

struct LFBarMarketDataField
{
	char_9   	TradingDay;            //交易日
	char_31  	InstrumentID;          //合约代码
    char_9   	ExchangeID;			   //交易所代码
	int64_t   	UpperLimitPrice;       //涨停板价
	int64_t   	LowerLimitPrice;       //跌停板价
	char_13  	StartUpdateTime;       //首tick修改时间
	int64_t     StartUpdateMillisec;   //首tick最后修改毫秒
	char_13  	EndUpdateTime;         //尾tick最后修改时间
	int64_t     EndUpdateMillisec;     //尾tick最后修改毫秒
	int			PeriodMillisec;        //周期（毫秒）
	int64_t   	Open;                  //开
	int64_t   	Close;                 //收
	int64_t   	Low;                   //低
	int64_t   	High;                  //高
	uint64_t   	Volume;                //区间交易量
	uint64_t   	StartVolume;           //初始总交易量
    int64_t         BestBidPrice;      
    int64_t         BestAskPrice;
    int Status;//状态码 0为正常/*quest3 edited by fxw*/
};

struct LFQryPositionField
{
	char_11  	BrokerID;              //经纪公司代码
	char_19  	InvestorID;            //投资者代码
	char_31  	InstrumentID;          //合约代码
	char_9   	ExchangeID;            //交易所代码
};

struct LFRspPositionField
{
	char_31              	InstrumentID;          //合约代码
    uint64_t                  	YdPosition;            //上日持仓
	uint64_t                  	Position;              //总持仓
	char_11              	BrokerID;              //经纪公司代码
	char_19              	InvestorID;            //投资者代码
	int64_t               	PositionCost;          //持仓成本
	LfHedgeFlagType      	HedgeFlag;             //投机套保标志
	LfPosiDirectionType  	PosiDirection;         //持仓多空方向
};

struct LFInputOrderField
{
	char_11                    	BrokerID;              //经纪公司代码
	char_16                    	UserID;                //用户代码
	char_19                    	InvestorID;            //投资者代码
	char_21                    	BusinessUnit;          //业务单元
	char_9                     	ExchangeID;            //交易所代码
	char_31                    	InstrumentID;          //合约代码
	char_21                    	OrderRef;              //报单引用
	int64_t                     	LimitPrice;            //价格
	uint64_t                        Volume;                //数量
	uint64_t                        MinVolume;             //最小成交量
	LfTimeConditionType        	TimeCondition;         //有效期类型
	LfVolumeConditionType      	VolumeCondition;       //成交量类型
	LfOrderPriceTypeType       	OrderPriceType;        //报单价格条件
	LfDirectionType            	Direction;             //买卖方向
	LfOffsetFlagType           	OffsetFlag;            //开平标志
	LfHedgeFlagType            	HedgeFlag;             //投机套保标志
	LfForceCloseReasonType     	ForceCloseReason;      //强平原因
	double                     	StopPrice;             //止损价
	int                        	IsAutoSuspend;         //自动挂起标志
	LfContingentConditionType  	ContingentCondition;   //触发条件
	char_64                    	MiscInfo;              //委托自定义标签
	uint64_t			MassOrderSeqId;
	int				MassOrderIndex;
	int				MassOrderTotalNum;
};

struct LFRtnOrderField
{
	char_11                	BrokerID;              //经纪公司代码
	char_16                	UserID;                //用户代码
	char_11                	ParticipantID;         //会员代码
	char_19                	InvestorID;            //投资者代码
	char_21                	BusinessUnit;          //业务单元
	char_31                	InstrumentID;          //合约代码
	char_21                	OrderRef;              //报单引用
	char_11                	ExchangeID;            //交易所代码
	int64_t                 LimitPrice;            //价格
	uint64_t                VolumeTraded;          //今成交数量
	uint64_t                VolumeTotal;           //剩余数量
	uint64_t                VolumeTotalOriginal;   //数量
	LfTimeConditionType    	TimeCondition;         //有效期类型
	LfVolumeConditionType  	VolumeCondition;       //成交量类型
	LfOrderPriceTypeType   	OrderPriceType;        //报单价格条件
	LfDirectionType        	Direction;             //买卖方向
	LfOffsetFlagType       	OffsetFlag;            //开平标志
	LfHedgeFlagType        	HedgeFlag;             //投机套保标志
	LfOrderStatusType      	OrderStatus;           //报单状态
	int                    	RequestID;             //请求编号
};

struct LFRtnTradeField
{
	char_11           	BrokerID;              //经纪公司代码
	char_16           	UserID;                //用户代码
	char_19           	InvestorID;            //投资者代码
	char_21           	BusinessUnit;          //业务单元
	char_31           	InstrumentID;          //合约代码
	char_21           	OrderRef;              //报单引用
	char_11           	ExchangeID;            //交易所代码
	char_21           	TradeID;               //成交编号
	char_31           	OrderSysID;            //报单编号
	char_11           	ParticipantID;         //会员代码
	char_21           	ClientID;              //客户代码
	int64_t            	Price;                 //价格
	uint64_t            Volume;                //数量
	char_13           	TradingDay;            //交易日
	char_13           	TradeTime;             //成交时间
	LfDirectionType   	Direction;             //买卖方向
	LfOffsetFlagType  	OffsetFlag;            //开平标志
	LfHedgeFlagType   	HedgeFlag;             //投机套保标志
};

struct LFOrderActionField
{
	char_11  	BrokerID;              //经纪公司代码
	char_19  	InvestorID;            //投资者代码
	char_31  	InstrumentID;          //合约代码
	char_11  	ExchangeID;            //交易所代码
	char_16  	UserID;                //用户代码
	char_21  	OrderRef;              //报单引用
	char_31  	OrderSysID;            //报单编号
	int      	RequestID;             //请求编号
	char     	ActionFlag;            //报单操作标志
	int64_t   	LimitPrice;            //价格
	uint64_t    VolumeChange;          //数量变化
	int      	KfOrderID;             //Kf系统内订单ID	
	char_64     MiscInfo;              //委托自定义标签
	uint64_t	MassOrderSeqId;
	int			MassOrderIndex;
	int			MassOrderTotalNum;
};

struct LFQryAccountField
{
	char_11  	BrokerID;              //经纪公司代码
	char_19  	InvestorID;            //投资者代码
};

struct LFRspAccountField
{
	char_11  	BrokerID;              //经纪公司代码
	char_19  	InvestorID;            //投资者代码
	double   	PreMortgage;           //上次质押金额
	double   	PreCredit;             //上次信用额度
	double   	PreDeposit;            //上次存款额
	double   	preBalance;            //上次结算准备金
	double   	PreMargin;             //上次占用的保证金
	double   	Deposit;               //入金金额
	double   	Withdraw;              //出金金额
	double   	FrozenMargin;          //冻结的保证金（报单未成交冻结的保证金）
	double   	FrozenCash;            //冻结的资金（报单未成交冻结的总资金）
	double   	FrozenCommission;      //冻结的手续费（报单未成交冻结的手续费）
	double   	CurrMargin;            //当前保证金总额
	double   	CashIn;                //资金差额
	double   	Commission;            //手续费
	double   	CloseProfit;           //平仓盈亏
	double   	PositionProfit;        //持仓盈亏
	double   	Balance;               //结算准备金
	double   	Available;             //可用资金
	double   	WithdrawQuota;         //可取资金
	double   	Reserve;               //基本准备金
	char_9   	TradingDay;            //交易日
	double   	Credit;                //信用额度
	double   	Mortgage;              //质押金额
	double   	ExchangeMargin;        //交易所保证金
	double   	DeliveryMargin;        //投资者交割保证金
	double   	ExchangeDeliveryMargin;   //交易所交割保证金
	double   	ReserveBalance;        //保底期货结算准备金
	double   	Equity;                //当日权益
	double   	MarketValue;           //账户市值
};

#endif
