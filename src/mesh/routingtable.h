#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

#include "route.h"
#include <map>

namespace iom {

    class RoutingTable
    {
    public:
       RoutingTable();
       /**
        * @brief Removes all expired routes from the routing table
        */
       void clean();
       /**
        * @brief Returns the route to destination
        * @param destination
        * @throws FailException if no route to destination exists
        * @return
        */
       boost::shared_ptr<Route> getRoute(const Address& destination);
       /**
        * @brief Removes the route to destination
        * @param destination
        */
       void invalidateRoute(const Address& destination);
       /**
        * @brief Adds a route to the routing table
        * @param route
        */
       void addRoute(boost::shared_ptr<Route> route);

    private:
       std::map<Address,boost::shared_ptr<Route> > routes;
    };
}

#endif // ROUTINGTABLE_H
