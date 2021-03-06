# Copyright [2017] [taurus.ai]
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
PROJECT(wingchuntd)

SET(COMMON_LIBRARIES journal kflog monitorapi)
SET(TD_SOURCE_FILES  ITDEngine.cpp ITDEngine.h ../base/IEngine.cpp ../base/IEngine.h ../base/TDUserStruct.cpp ../base/TDUserStruct.h)

#SET(CTP_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineCTP.cpp TDEngineCTP.h)
#ADD_LIBRARY(ctptd SHARED ${CTP_SOURCE_FILES})
#TARGET_LINK_LIBRARIES(ctptd journal kflog thosttraderapi ${Boost_LIBRARIES})
#INSTALL(TARGETS ctptd DESTINATION "lib/wingchun")

#SET(XTP_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineXTP.cpp TDEngineXTP.h)
#ADD_LIBRARY(xtptd SHARED ${XTP_SOURCE_FILES})
#TARGET_LINK_LIBRARIES(xtptd journal kflog xtptraderapi ${Boost_LIBRARIES})
#INSTALL(TARGETS xtptd DESTINATION "lib/wingchun")

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/utils/cpr/include ${CMAKE_SOURCE_DIR}/utils/rapidjson/include/ ${CMAKE_SOURCE_DIR}/utils/libwebsockets/include)

SET(BINANCE_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineBinance.cpp TDEngineBinance.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h ../base/InterfaceMgr.cpp ../base/InterfaceMgr.h)
ADD_LIBRARY(binancetd SHARED ${BINANCE_SOURCE_FILES})
TARGET_LINK_LIBRARIES(binancetd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so)
INSTALL(TARGETS binancetd DESTINATION "lib/wingchun")


SET(COINMEX_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineCoinmex.cpp TDEngineCoinmex.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(coinmextd SHARED ${COINMEX_SOURCE_FILES})
TARGET_LINK_LIBRARIES(coinmextd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS coinmextd DESTINATION "lib/wingchun")


SET(MOCK_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineMock.cpp TDEngineMock.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(mocktd SHARED ${MOCK_SOURCE_FILES})
TARGET_LINK_LIBRARIES(mocktd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so)
INSTALL(TARGETS mocktd DESTINATION "lib/wingchun")


SET(BITMAX_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineBitmax.cpp TDEngineBitmax.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(bitmaxtd SHARED ${BITMAX_SOURCE_FILES})
TARGET_LINK_LIBRARIES(bitmaxtd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so)
INSTALL(TARGETS bitmaxtd DESTINATION "lib/wingchun")

SET(BITFINEX_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineBitfinex.cpp TDEngineBitfinex.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(bitfinextd SHARED ${BITFINEX_SOURCE_FILES})
TARGET_LINK_LIBRARIES(bitfinextd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS bitfinextd DESTINATION "lib/wingchun")

SET(BITTREX_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineBittrex.cpp TDEngineBittrex.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(bittrextd SHARED ${BITTREX_SOURCE_FILES})
TARGET_LINK_LIBRARIES(bittrextd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS bittrextd DESTINATION "lib/wingchun")

SET(BITMEX_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineBitmex.cpp TDEngineBitmex.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(bitmextd SHARED ${BITMEX_SOURCE_FILES})
TARGET_LINK_LIBRARIES(bitmextd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS bitmextd DESTINATION "lib/wingchun")


SET(HITBTC_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineHitBTC.cpp TDEngineHitBTC.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(hitbtctd SHARED ${HITBTC_SOURCE_FILES})
TARGET_LINK_LIBRARIES(hitbtctd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS hitbtctd DESTINATION "lib/wingchun")


SET(OCEANEX_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineOceanEx.cpp TDEngineOceanEx.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(oceanextd SHARED ${OCEANEX_SOURCE_FILES})
TARGET_LINK_LIBRARIES(oceanextd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS oceanextd DESTINATION "lib/wingchun")

SET(OCEANEXB_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineOceanEx2.cpp TDEngineOceanEx2.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(oceanexbtd SHARED ${OCEANEXB_SOURCE_FILES})
TARGET_LINK_LIBRARIES(oceanexbtd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS oceanexbtd DESTINATION "lib/wingchun")

SET(PROBIT_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineProbit.cpp TDEngineProbit.h  ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(probittd SHARED ${PROBIT_SOURCE_FILES})
TARGET_LINK_LIBRARIES(probittd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS probittd DESTINATION "lib/wingchun")

SET(BITHUMB_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineBithumb.cpp TDEngineBithumb.h  ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(bithumbtd SHARED ${BITHUMB_SOURCE_FILES})
TARGET_LINK_LIBRARIES(bithumbtd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so)
INSTALL(TARGETS bithumbtd DESTINATION "lib/wingchun")
SET(UPBIT_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineUpbit.cpp TDEngineUpbit.h  ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(upbittd SHARED ${UPBIT_SOURCE_FILES})
TARGET_LINK_LIBRARIES(upbittd journal kflog ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so ${CMAKE_SOURCE_DIR}/utils/uuid/lib/libuuid.so)
INSTALL(TARGETS upbittd DESTINATION "lib/wingchun")

SET(DAYBIT_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineDaybit.cpp TDEngineDaybit.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(daybittd SHARED ${DAYBIT_SOURCE_FILES})
TARGET_LINK_LIBRARIES(daybittd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS daybittd DESTINATION "lib/wingchun")

SET(KRAKEN_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineKraken.cpp TDEngineKraken.h  ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(krakentd SHARED ${KRAKEN_SOURCE_FILES})
TARGET_LINK_LIBRARIES(krakentd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS krakentd DESTINATION "lib/wingchun")

SET(BITFLYER_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineBitflyer.cpp TDEngineBitflyer.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h ../base/InterfaceMgr.cpp ../base/InterfaceMgr.h)
ADD_LIBRARY(bitflyertd SHARED ${BITFLYER_SOURCE_FILES})
TARGET_LINK_LIBRARIES(bitflyertd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so)
INSTALL(TARGETS bitflyertd DESTINATION "lib/wingchun")

SET(POLONIEX_SOURCE_FILES ${TD_SOURCE_FILES} TDEnginePoloniex.cpp TDEnginePoloniex.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(poloniextd SHARED ${POLONIEX_SOURCE_FILES})
TARGET_LINK_LIBRARIES(poloniextd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS poloniextd DESTINATION "lib/wingchun")

SET(KUCOIN_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineKuCoin.cpp TDEngineKuCoin.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(kucointd SHARED ${KUCOIN_SOURCE_FILES})
TARGET_LINK_LIBRARIES(kucointd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS kucointd DESTINATION "lib/wingchun")

SET(EMX_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineEmx.cpp TDEngineEmx.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(emxtd SHARED ${EMX_SOURCE_FILES})
TARGET_LINK_LIBRARIES(emxtd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS emxtd DESTINATION "lib/wingchun")

SET(HUOBI_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineHuobi.cpp TDEngineHuobi.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h ../base/InterfaceMgr.cpp ../base/InterfaceMgr.h)
ADD_LIBRARY(huobitd SHARED ${HUOBI_SOURCE_FILES})
TARGET_LINK_LIBRARIES(huobitd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so)
INSTALL(TARGETS huobitd DESTINATION "lib/wingchun")

SET(BITSTAMP_SOURCE_FILES ${TD_SOURCE_FILES} TDEngineBitstamp.cpp TDEngineBitstamp.h ../base/CoinPairWhiteList.cpp ../base/CoinPairWhiteList.h)
ADD_LIBRARY(bitstamptd SHARED ${BITSTAMP_SOURCE_FILES})
TARGET_LINK_LIBRARIES(bitstamptd ${COMMON_LIBRARIES} ${Boost_LIBRARIES} ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcpr.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcurl.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libcrypto.so ${CMAKE_SOURCE_DIR}/utils/cpr/lib/libssl.so ${CMAKE_SOURCE_DIR}/utils/libwebsockets/lib/libwebsockets.so)
INSTALL(TARGETS bitstamptd DESTINATION "lib/wingchun")

