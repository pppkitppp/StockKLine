﻿#include "strategyjiaotansimple2.h"
#include "Utility.h"
#include "BacktestingTradeGateway.h"

StrategyJiaoTanSimple2::StrategyJiaoTanSimple2()
    : closeVecPtr_(std::make_shared<std::vector<double>>()),
    lastDiff_(0.0), currentDiff_(0.0),
    lastDiff1_(0.0), currentDiff1_(0.0),
    maClose10Tracker(10, closeVecPtr_),
    maClose17Tracker(17, closeVecPtr_),
    maClose15Tracker(15, closeVecPtr_),
    maClose50Tracker(50, closeVecPtr_),
    maClose170Tracker(170, closeVecPtr_),
    maClose190Tracker(190, closeVecPtr_),
    backtestingConfig(BacktestingConfig::instance())
{

}

void StrategyJiaoTanSimple2::init(TradeGatewayPtr pTradeGateway)
{
    this->tradeGatewayPtr_ = pTradeGateway;
}

void StrategyJiaoTanSimple2::onTick(Tick &tick)
{

}

void StrategyJiaoTanSimple2::onBar(KLineDataType &bar)
{
    double close = bar.close_price;
    closeVecPtr_->push_back(close);
    double maClose190 = maClose190Tracker.value();
    double maClose170 = maClose170Tracker.value();
    double maClose50 = maClose50Tracker.value();
    double maClose15 = maClose15Tracker.value();
    double maClose17 = maClose17Tracker.value();
    double maClose10 = maClose10Tracker.value();
    double diff = maClose15 - maClose170;
    lastDiff_ = currentDiff_;
    currentDiff_ = diff;
    double diff1 = maClose10 - maClose190;
    lastDiff1_ = currentDiff1_;
    currentDiff1_ = diff1;
    lastMA15_ = currentMA15_;
    currentMA15_ = maClose15;
    lastMA50_ = currentMA50_;
    currentMA50_ = maClose50;

    if (closeVecPtr_->size() <= 250) {
        return;
    }

    if (diff1 < 50.0 && close > maClose190 && diff > 0.0) {
        BPK(backtestingConfig->baseLot);
    }
    else if ( (lastDiff_ > 0.0 && currentDiff_ < 0.0) ||
              (lastDiff_ > 0.0 && currentDiff_ == 0.0) ||
              (lastDiff_ == 0.0 && currentDiff_ < 0.0)
            ) {
        SP(backtestingConfig->baseLot);
    }
    else if (diff1 > 70.0 &&
             ((lastMA50_ < lastMA15_ && currentMA50_ > currentMA15_) ||
              (lastMA50_ < lastMA15_ && currentMA50_ == currentMA15_) ||
              (lastMA50_ == lastMA15_ && currentMA50_ > currentMA15_)
              )
             ) {
        SP(backtestingConfig->baseLot);
    }
    else if (diff1 > 700.0) {
        SP(backtestingConfig->baseLot);
    }
    else if (diff1 > -50.0 && close < maClose190 && diff < 0.0) {
        SPK(backtestingConfig->baseLot);
    }
    else if ( (lastDiff_ < 0.0 && currentDiff_ > 0.0) ||
              (lastDiff_ < 0.0 && currentDiff_ == 0.0) ||
              (lastDiff_ == 0.0 && currentDiff_ > 0.0)
            ) {
        BP(backtestingConfig->baseLot);
    }
    else if (diff1 < -70.0 &&
             ((lastMA15_ < lastMA50_ && currentMA15_ > currentMA50_) ||
              (lastMA15_ < lastMA50_ && currentMA15_ == currentMA50_) ||
              (lastMA15_ == lastMA50_ && currentMA15_ > currentMA50_)
              )
             ) {
        BP(backtestingConfig->baseLot);
    }
    else if (diff1 < -700.0) {
        BP(backtestingConfig->baseLot);
    }
}

void StrategyJiaoTanSimple2::openPosition(int volume)
{
    auto gateWay = std::static_pointer_cast<BacktestingTradeGateway>(tradeGatewayPtr_);
    int longTradeCount = gateWay->getLongTradeCount();
    int shortTradeCount = gateWay->getShortTradeCount();
    if (longTradeCount > 0) {
        BK(volume);
    }
    else if (shortTradeCount > 0) {
        SK(volume);
    }
}

void StrategyJiaoTanSimple2::closePosition(int volume)
{
    auto gateWay = std::static_pointer_cast<BacktestingTradeGateway>(tradeGatewayPtr_);
    int longTradeCount = gateWay->getLongTradeCount();
    int shortTradeCount = gateWay->getShortTradeCount();
    if (longTradeCount > 0) {
        if (volume > longTradeCount) {
            SP(longTradeCount);
        }
        else {
            SP(volume);
        }
    }
    else if (shortTradeCount > 0) {
        if (volume > shortTradeCount) {
            BP(shortTradeCount);
        }
        else {
            BP(volume);
        }
    }
}

void StrategyJiaoTanSimple2::closeAllPosition()
{
    closePosition(std::numeric_limits<int>::max());
}

void StrategyJiaoTanSimple2::BPK(int volume)
{
    tradeGatewayPtr_->closeShortAndOpenLong("instrumentID", volume);
}

void StrategyJiaoTanSimple2::SPK(int volume)
{
    tradeGatewayPtr_->closeLongAndOpenShort("instrumentID", volume);
}

void StrategyJiaoTanSimple2::SP(int volume)
{
    tradeGatewayPtr_->closeLong("instrumentID", volume);
}

void StrategyJiaoTanSimple2::BP(int volume)
{
    tradeGatewayPtr_->closeShort("instrumentID", volume);
}

void StrategyJiaoTanSimple2::SK(int volume)
{
    tradeGatewayPtr_->openShort("instrumentID", volume);
}

void StrategyJiaoTanSimple2::BK(int volume)
{
    tradeGatewayPtr_->openLong("instrumentID", volume);
}

int StrategyJiaoTanSimple2::getCloseMinute(KLineDataType &bar)
{
    int minutesOfDay = getMinutesOfDay(bar.update_time);

    int minutesToClose = (10 * 60 + 15) - minutesOfDay;
    if (minutesToClose > 0) return minutesToClose;

    minutesToClose = (11 * 60 + 30) - minutesOfDay;
    if (minutesToClose > 0) return minutesToClose;

    minutesToClose = (15 * 60) - minutesOfDay;
    if (minutesToClose > 0) return minutesToClose;

    minutesToClose = (23 * 60 + 30) - minutesOfDay;
    if (minutesToClose > 0) return minutesToClose;

    return minutesToClose;
}

void StrategyJiaoTanSimple2::onTrade(Trade &trade)
{

}

void StrategyJiaoTanSimple2::onOrder(OrderRsp &order)
{

}

std::string StrategyJiaoTanSimple2::name()
{
    return "TieKuangShi Extended Strategy";
}

std::string StrategyJiaoTanSimple2::instrumentType()
{
    return "i";
}
