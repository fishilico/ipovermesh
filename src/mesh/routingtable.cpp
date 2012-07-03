#include "routingtable.h"
#include "../core/common.h"

namespace iom {

    RoutingTable::RoutingTable()
    {
    }

    void RoutingTable::clean()
    {
        boost::unique_lock<boost::shared_mutex> lock(mut);
        std::map<Address, boost::shared_ptr<Route> >::iterator it;
        for(it = routes.begin(); it != routes.end(); it++)
        {
            if(it->second->hasExpired())
                routes.erase(it);
        }
    }

    boost::shared_ptr<Route> RoutingTable::getRoute(const Address &destination)
    {
        boost::upgrade_lock<boost::shared_mutex> lock(mut);
        std::map<Address, boost::shared_ptr<Route> >::iterator it = routes.find(destination);
        if(it == routes.end())
            return boost::shared_ptr<Route>();
        if(it->second->hasExpired())
        {
            boost::upgrade_to_unique_lock<boost::shared_mutex> ulock(lock);
            routes.erase(it);
            return boost::shared_ptr<Route>();
        }
        return it->second;
    }

    void RoutingTable::invalidateRoute(const Address &destination)
    {
        if (MESH_DEBUG_RT)
            log::debug << "[Route] Invalidate route to " << destination << log::endl;

        boost::unique_lock<boost::shared_mutex> lock(mut);
        std::map<Address, boost::shared_ptr<Route> >::iterator it = routes.find(destination);
        if(it != routes.end())
            routes.erase(it);
    }

    void RoutingTable::addRoute(boost::shared_ptr<Route> route)
    {
        if (MESH_DEBUG_RT)
            log::debug << "[Route] Add route to " << route->destination << " via "
                << route->nextHop << log::endl;

        boost::unique_lock<boost::shared_mutex> lock(mut);
        routes.insert(std::pair<Address, boost::shared_ptr<Route> >(route->destination, route));
    }
}
