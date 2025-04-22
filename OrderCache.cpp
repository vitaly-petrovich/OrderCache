#include <stdexcept>
#include "OrderCache.h"
#include "gtest/gtest.h"

static constexpr std::string_view BUY = "Buy";
static constexpr std::string_view SELL = "Sell";

void OrderCache::addOrder(Order order) {
  if(order.orderId().empty())
    throw std::invalid_argument("Error: order ID is empty!");
  if(order.securityId().empty())
    throw std::invalid_argument("Error: security ID is empty!");
  if(order.user().empty())
    throw std::invalid_argument("Error: user ID is empty!");
  if(order.company().empty())
    throw std::invalid_argument("Error: company name is empty!");
  if(order.side().empty())
    throw std::invalid_argument("Error: side is empty!");
  if(!order.qty())
    throw std::invalid_argument("Error: qty is zero!");  
  if(m_orderIds.find(order.orderId()) != m_orderIds.end())
    throw std::runtime_error("Error: order ID have already exist!");
  m_orderIds.insert(order.orderId());

  if(order.side() == BUY)
    m_buyOrders[order.securityId()].push_back({ order });
  else if(order.side() == SELL)
    m_sellOrders[order.securityId()].push_back({ order });
  else
    throw std::invalid_argument("Error:invalid side!");
}

void OrderCache::cancelOrder(const std::string& orderId) {
  if(orderId.empty())
    throw std::invalid_argument("Error: order ID is empty!");
    
  m_orderIds.erase(orderId);
  if(cancelOrder(orderId, m_buyOrders))
    return;
  if(cancelOrder(orderId, m_sellOrders))
    return;
}

void OrderCache::cancelOrdersForUser(const std::string& user) {
  if(user.empty())
    throw std::invalid_argument("Error: user is empty!");

  cancelOrdersForUser(user, m_buyOrders, m_orderIds);
  cancelOrdersForUser(user, m_sellOrders, m_orderIds);
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) {
  if(securityId.empty())
    throw std::invalid_argument("Error: security ID is empty!");
  if(!minQty)
    throw std::invalid_argument("Error: minQty is zero!");

  cancelOrdersForSecIdWithMinimumQty(securityId, minQty, m_buyOrders, m_orderIds);
  cancelOrdersForSecIdWithMinimumQty(securityId, minQty, m_sellOrders, m_orderIds);
}

unsigned int OrderCache::getMatchingSizeForSecurity(const std::string& securityId) {
  if(securityId.empty())
    throw std::invalid_argument("Error: security ID is empty!");

  unsigned int totalQty = 0;
  auto& vecSell = m_sellOrders[securityId];
  for(std::size_t i = 0; i < vecSell.size(); i++)
  {
    auto& vecBuy = m_buyOrders[securityId];
    for(std::size_t j = 0; j < vecBuy.size(); j++)
    {
      if(vecSell[i].company() == vecBuy[j].company())
        continue;

      if(vecSell[i].currentQty >= vecBuy[j].currentQty)
      {
        totalQty += vecBuy[j].currentQty;
        vecSell[i].currentQty -= vecBuy[j].currentQty;
        erase(vecBuy, j);
        if(!vecSell[i].currentQty)
        {
          erase(vecSell, i);
          break;
        }
      }
      else
      {
        totalQty += vecSell[i].currentQty;
        vecBuy[j].currentQty -= vecSell[i].currentQty;
        erase(vecSell, i);
        break;
      }
    }
  }
  return totalQty;
}

std::vector<Order> OrderCache::getAllOrders() const {
  std::vector<Order> orders;
  orders.reserve(m_buyOrders.size() + m_sellOrders.size());
  for(auto& pair: m_buyOrders)
  {
    for(std::size_t i = 0; i < pair.second.size(); i++)
      orders.push_back(pair.second[i]);
  }
  for(auto& pair: m_sellOrders)
  {
    for(std::size_t i = 0; i < pair.second.size(); i++)
      orders.push_back(pair.second[i]);
  }
  return orders;
}

////------------------------  PRIVATE -------------------------------------------

void OrderCache::erase(std::vector<OrderExpander>& vctr, std::size_t& index)
{
  if(index < vctr.size()-1)
    std::swap(vctr[index--], vctr[vctr.size()-1]);
  vctr.erase(--vctr.end());
}

bool OrderCache::cancelOrder(const std::string& orderId
    , MapOrders& mapOrders)
{
  for(auto& pair: mapOrders)
  {
    for(std::size_t i = 0; i < pair.second.size(); i++)
    {
      if(pair.second[i].orderId() != orderId)
        continue;

      erase(pair.second, i);
      return true;
    }
  }
  return false;
}

void OrderCache::cancelOrdersForUser(const std::string& user
    , MapOrders& mapOrders
    , OrderIds& orderIds)
{
  for(auto& pair: mapOrders)
  {
    for(std::size_t i = 0; i < pair.second.size(); i++)
    {
      if(pair.second[i].user() != user)
        continue;

      orderIds.erase(pair.second[i].orderId());
      erase(pair.second, i);
    }
  }
}

void OrderCache::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId
  , unsigned int minQty
  , MapOrders& mapOrders
  , OrderIds& orderIds)
{
  auto& vec = mapOrders[securityId];
  for(std::size_t i = 0; i < vec.size(); i++)
  {
    if(vec[i].qty() < minQty)
      continue;

    orderIds.erase(vec[i].orderId());
    erase(vec, i);
  }
}

/******************************   MY RESULT   ******************************
[==========] Running 46 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 46 tests from OrderCacheTest
[     INFO ] Test version: 1.4
[     INFO ] 1 NCU = 9ms
[ RUN      ] OrderCacheTest.ThirdParty_Dependencies_BoostNotUsed
[       OK ] OrderCacheTest.ThirdParty_Dependencies_BoostNotUsed (0 ms)
[ RUN      ] OrderCacheTest.BasicOperations_AddOrder_AddOrderWithoutException
[       OK ] OrderCacheTest.BasicOperations_AddOrder_AddOrderWithoutException (0 ms)
[ RUN      ] OrderCacheTest.BasicOperations_GetAllOrders_ReturnsCorrectNumberOfOrders
[       OK ] OrderCacheTest.BasicOperations_GetAllOrders_ReturnsCorrectNumberOfOrders (0 ms)
[ RUN      ] OrderCacheTest.BasicOperations_CancelOrder_RemovesSpecificOrderById
[       OK ] OrderCacheTest.BasicOperations_CancelOrder_RemovesSpecificOrderById (6 ms)
[ RUN      ] OrderCacheTest.BasicOperations_CancelOrdersForUser_RemovesAllUserOrders
[       OK ] OrderCacheTest.BasicOperations_CancelOrdersForUser_RemovesAllUserOrders (0 ms)
[ RUN      ] OrderCacheTest.BasicOperations_CancelOrdersWithMinimumQty_RemovesQualifyingOrders
[       OK ] OrderCacheTest.BasicOperations_CancelOrdersWithMinimumQty_RemovesQualifyingOrders (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_ReadmeExample1_MatchesCorrectly
[       OK ] OrderCacheTest.MatchingSize_ReadmeExample1_MatchesCorrectly (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_ReadmeExample2_MatchesCorrectly
[       OK ] OrderCacheTest.MatchingSize_ReadmeExample2_MatchesCorrectly (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_ReadmeExample3_MatchesCorrectly
[       OK ] OrderCacheTest.MatchingSize_ReadmeExample3_MatchesCorrectly (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_OneToMany_MatchesMultipleSellers
[       OK ] OrderCacheTest.MatchingSize_OneToMany_MatchesMultipleSellers (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_ComplexCombinations_MatchesCorrectly
[       OK ] OrderCacheTest.MatchingSize_ComplexCombinations_MatchesCorrectly (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_SameCompany_DoesNotMatch
[       OK ] OrderCacheTest.MatchingSize_SameCompany_DoesNotMatch (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_LargeBuyer_MatchesWithMultipleSmallSellers
[       OK ] OrderCacheTest.MatchingSize_LargeBuyer_MatchesWithMultipleSmallSellers (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_ManyToMany_MatchesBidirectionally
[       OK ] OrderCacheTest.MatchingSize_ManyToMany_MatchesBidirectionally (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_OnlyBuyOrders_ReturnsZero
[       OK ] OrderCacheTest.MatchingSize_OnlyBuyOrders_ReturnsZero (0 ms)
[ RUN      ] OrderCacheTest.MatchingSize_OnlySellOrders_ReturnsZero
[       OK ] OrderCacheTest.MatchingSize_OnlySellOrders_ReturnsZero (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_AddOrder_EmptyOrderIdThrowsException
[       OK ] OrderCacheTest.EdgeCases_AddOrder_EmptyOrderIdThrowsException (4 ms)
[ RUN      ] OrderCacheTest.EdgeCases_AddOrder_EmptySecurityIdThrowsException
[       OK ] OrderCacheTest.EdgeCases_AddOrder_EmptySecurityIdThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_AddOrder_EmptyUserIdThrowsException
[       OK ] OrderCacheTest.EdgeCases_AddOrder_EmptyUserIdThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_AddOrder_EmptyCompanyThrowsException
[       OK ] OrderCacheTest.EdgeCases_AddOrder_EmptyCompanyThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_AddOrder_EmptySideThrowsException
[       OK ] OrderCacheTest.EdgeCases_AddOrder_EmptySideThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_AddOrder_InvalidSideThrowsException
[       OK ] OrderCacheTest.EdgeCases_AddOrder_InvalidSideThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_AddOrder_ZeroQuantityThrowsException
[       OK ] OrderCacheTest.EdgeCases_AddOrder_ZeroQuantityThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_AddOrder_ReplaceExistingOrderThrowsException
[       OK ] OrderCacheTest.EdgeCases_AddOrder_ReplaceExistingOrderThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrder_EmptyOrderIdThrowsException
[       OK ] OrderCacheTest.EdgeCases_CancelOrder_EmptyOrderIdThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrder_NonexistentOrderSilentlyReturn
[       OK ] OrderCacheTest.EdgeCases_CancelOrder_NonexistentOrderSilentlyReturn (0 ms)
[ RUN      ] OrderCacheTest.EddgeCases_CancelOrder_AddNewOrderWithSameOrderIdShouldSucceed
[       OK ] OrderCacheTest.EddgeCases_CancelOrder_AddNewOrderWithSameOrderIdShouldSucceed (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrder_CancelThenAddNewOrdersForSameUserShouldSucceed
[       OK ] OrderCacheTest.EdgeCases_CancelOrder_CancelThenAddNewOrdersForSameUserShouldSucceed (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrdersForUser_EmptyUserThrowsException
[       OK ] OrderCacheTest.EdgeCases_CancelOrdersForUser_EmptyUserThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrdersForUser_NoOrdersFoundSilentlyReturn
[       OK ] OrderCacheTest.EdgeCases_CancelOrdersForUser_NoOrdersFoundSilentlyReturn (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrdersForUser_AcrossMultipleSecuritiesShouldRemoveAll
[       OK ] OrderCacheTest.EdgeCases_CancelOrdersForUser_AcrossMultipleSecuritiesShouldRemoveAll (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_EmptySecurityIdThrowsException
[       OK ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_EmptySecurityIdThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_ZeroQuantityThrowsException
[       OK ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_ZeroQuantityThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_NonOrdersFoundSilentlyReturn
[       OK ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_NonOrdersFoundSilentlyReturn (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_LowQuantityOrdersShouldRemain
[       OK ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_LowQuantityOrdersShouldRemain (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_CancelThenAddNewOrdersForSameSecurityShouldSucceed
[       OK ] OrderCacheTest.EdgeCases_CancelOrdersForSecIdWithMinimumQty_CancelThenAddNewOrdersForSameSecurityShouldSucceed (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_GetMatchingSizeForSecurity_EmptySecurityThrowsException
[       OK ] OrderCacheTest.EdgeCases_GetMatchingSizeForSecurity_EmptySecurityThrowsException (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_GetMatchingSizeForSecurity_NonexistentSecurityReturnsZero
[       OK ] OrderCacheTest.EdgeCases_GetMatchingSizeForSecurity_NonexistentSecurityReturnsZero (0 ms)
[ RUN      ] OrderCacheTest.EdgeCases_GetAllOrders_ResultNotModifiableExternally
[       OK ] OrderCacheTest.EdgeCases_GetAllOrders_ResultNotModifiableExternally (0 ms)
[ RUN      ] OrderCacheTest.Performance_SmallDataset_1KOrders
[     INFO ] Matched 1000 orders in 0.222222 NCUs (2ms)
[       OK ] OrderCacheTest.Performance_SmallDataset_1KOrders (3 ms)
[ RUN      ] OrderCacheTest.Performance_SmallDataset_5KOrders
[     INFO ] Matched 5000 orders in 1.11111 NCUs (10ms)
[       OK ] OrderCacheTest.Performance_SmallDataset_5KOrders (15 ms)
[ RUN      ] OrderCacheTest.Performance_MediumDataset_10KOrders
[     INFO ] Matched 10000 orders in 2 NCUs (18ms)
[       OK ] OrderCacheTest.Performance_MediumDataset_10KOrders (28 ms)
[ RUN      ] OrderCacheTest.Performance_MediumDataset_50KOrders
[     INFO ] Matched 50000 orders in 11.2222 NCUs (101ms)
[       OK ] OrderCacheTest.Performance_MediumDataset_50KOrders (143 ms)
[ RUN      ] OrderCacheTest.Performance_LargeDataset_100KOrders
[     INFO ] Matched 100000 orders in 21.3333 NCUs (192ms)
[       OK ] OrderCacheTest.Performance_LargeDataset_100KOrders (271 ms)
[ RUN      ] OrderCacheTest.Performance_LargeDataset_500KOrders
[     INFO ] Matched 500000 orders in 117.333 NCUs (1056ms)
[       OK ] OrderCacheTest.Performance_LargeDataset_500KOrders (1469 ms)
[ RUN      ] OrderCacheTest.Performance_VeryLargeDataset_1MOrders
[     INFO ] Matched 1000000 orders in 242 NCUs (2178ms)
[       OK ] OrderCacheTest.Performance_VeryLargeDataset_1MOrders (3047 ms)
[----------] 46 tests from OrderCacheTest (5010 ms total)

[----------] Global test environment tear-down
[==========] 46 tests from 1 test suite ran. (5019 ms total)
[  PASSED  ] 46 tests.
*****************************************************************************/