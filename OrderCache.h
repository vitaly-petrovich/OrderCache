#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

class Order
{

 public:

  // do not alter signature of this constructor
  Order(
      const std::string& ordId,
      const std::string& secId,
      const std::string& side,
      const unsigned int qty,
      const std::string& user,
      const std::string& company)
      : m_orderId(ordId),
        m_securityId(secId),
        m_side(side),
        m_qty(qty),
        m_user(user),
        m_company(company) { }

  // do not alter these accessor methods
  std::string orderId() const    { return m_orderId; }
  std::string securityId() const { return m_securityId; }
  std::string side() const       { return m_side; }
  std::string user() const       { return m_user; }
  std::string company() const    { return m_company; }
  unsigned int qty() const       { return m_qty; }

 private:

  // use the below to hold the order data
  // do not remove the these member variables
  std::string m_orderId;     // unique order id
  std::string m_securityId;  // security identifier
  std::string m_side;        // side of the order, eg Buy or Sell
  unsigned int m_qty;        // qty for this order
  std::string m_user;        // user name who owns this order
  std::string m_company;     // company for user

};

struct OrderExpander : public Order
{
   OrderExpander(Order& order)
      : Order(std::move(order))
      , currentQty(qty()) { }

   unsigned int currentQty;
};

// Provide an implementation for the OrderCacheInterface interface class.
// Your implementation class should hold all relevant data structures you think
// are needed.
class OrderCacheInterface
{

 public:

  // implement the 6 methods below, do not alter signatures

  // add order to the cache
  virtual void addOrder(Order order) = 0;

  // remove order with this unique order id from the cache
  virtual void cancelOrder(const std::string& orderId) = 0;

  // remove all orders in the cache for this user
  virtual void cancelOrdersForUser(const std::string& user) = 0;

  // remove all orders in the cache for this security with qty >= minQty
  virtual void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) = 0;

  // return the total qty that can match for the security id
  virtual unsigned int getMatchingSizeForSecurity(const std::string& securityId) = 0;

  // return all orders in cache in a vector
  virtual std::vector<Order> getAllOrders() const = 0;

};

// Todo: Your implementation of the OrderCache...
class OrderCache : public OrderCacheInterface
{
  using MapOrders = std::unordered_map<std::string, std::vector<OrderExpander>>;  
  using OrderIds = std::unordered_set<std::string>;

 public:

  void addOrder(Order order) override;

  void cancelOrder(const std::string& orderId) override;

  void cancelOrdersForUser(const std::string& user) override;

  void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) override;

  unsigned int getMatchingSizeForSecurity(const std::string& securityId) override;

  std::vector<Order> getAllOrders() const override;

 private:
   MapOrders m_buyOrders;
   MapOrders m_sellOrders;
   OrderIds m_orderIds;

   void erase(std::vector<OrderExpander>& vct, std::size_t& index);

   bool cancelOrder(const std::string& orderId
      , MapOrders& mapOrders);

   void cancelOrdersForUser(const std::string& user
      , MapOrders& mapOrders
      , OrderIds& orderIds);

   void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId
      , unsigned int minQty
      , MapOrders& mapOrders
      , OrderIds& orderIds);
};
