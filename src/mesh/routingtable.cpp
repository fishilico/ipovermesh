#include "routingtable.h"
#include "../core/common.h"

namespace iom {

    RoutingTable::RoutingTable()
    {
    }

    void RoutingTable::clean()
    {
        std::map<Address, boost::shared_ptr<Route> >::iterator it;

        for(it = routes.begin(); it != routes.end(); it++)
        {
            if(it->second->hasExpired())
                routes.erase(it);
        }
    }

    boost::shared_ptr<Route> RoutingTable::getRoute(const Address &destination)
    {
        std::map<Address, boost::shared_ptr<Route> >::iterator it = routes.find(destination);
        if(it == routes.end())
            return boost::shared_ptr<Route>();
        if(it->second->hasExpired())
        {
            routes.erase(it);
            return boost::shared_ptr<Route>();
        }
        return it->second;
    }

    void RoutingTable::invalidateRoute(const Address &destination)
    {
        std::map<Address, boost::shared_ptr<Route> >::iterator it = routes.find(destination);
        if(it != routes.end())
            routes.erase(it);
    }

    void RoutingTable::addRoute(boost::shared_ptr<Route> route)
    {
        routes.insert(std::pair<Address, boost::shared_ptr<Route> >(route->destination, route));
    }
}
