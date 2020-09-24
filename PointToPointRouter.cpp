#include "provided.h"
#include "ExpandableHashMap.h"
#include <utility>
#include <list>
#include <set>
#include <vector>
using namespace std;

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;
    
private:
    const StreetMap* m_stmap;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm): m_stmap(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    route.clear();
    totalDistanceTravelled = 0;
    if (start == end)
    {
        return DELIVERY_SUCCESS;
    }
    
    // Check if the beginning and ending coordinate are in the map:
    vector<StreetSegment> test1,test2;
    m_stmap->getSegmentsThatStartWith(start,test1);
    m_stmap->getSegmentsThatStartWith(end,test2);
    if (test1.empty() || test2.empty())
        return BAD_COORD;
        
    ExpandableHashMap<GeoCoord,GeoCoord> closed_list;
    set<pair<double,pair<GeoCoord,GeoCoord>>> open_list;
    
    open_list.insert(make_pair(0.0,make_pair(start,start)));
    
    GeoCoord parent = start;
    double current_f = 0.0;
    
    while (!open_list.empty())
    {
        pair<double,pair<GeoCoord,GeoCoord>> top = *open_list.begin();
        open_list.erase(open_list.begin());
        
        parent = top.second.second;  // set parent to the second geocoord
        current_f = top.first;  // set the current f-value to be added to the f-value of the parent
        
        closed_list.associate(parent,top.second.first);
        /*
        if (parent != start)
        {
            //add the top of the priority queue into the route
            route.push_back(top.second);
            totalDistanceTravelled += distanceEarthMiles(top.second.start,top.second.end);
        }
        */
        
        if (parent == end)
        {
            GeoCoord current_geo = end;
            GeoCoord next_geo = *closed_list.find(end);
            
            while (current_geo != start)
            {
                totalDistanceTravelled += distanceEarthMiles(next_geo,current_geo);
                vector<StreetSegment> find_seg;
                m_stmap->getSegmentsThatStartWith(next_geo, find_seg);
                for (auto p = find_seg.begin(); p != find_seg.end(); p++)
                {
                    if ((*p).end == current_geo)
                    {
                        route.push_front((*p));
                    }
                }
                current_geo = next_geo;
                next_geo = *closed_list.find(current_geo);
            }
            // go through every geocoord and its parent
            // have to use find in hash map to find geocoord partner & streetsegment appropriate, then push front onto the list
            // add up everything
            
            return DELIVERY_SUCCESS;
        }
        
        vector<StreetSegment> possible_segs;
        m_stmap->getSegmentsThatStartWith(parent,possible_segs);
        for (auto p = possible_segs.begin(); p != possible_segs.end(); p++)
        {
            GeoCoord possible_geo = (*p).end;
            if (!closed_list.find(possible_geo)) // check if already in closed list
            {
                // calculate f-value
                double g = distanceEarthMiles(possible_geo,end);
                double f = g + current_f;
                // check if already in open list (change f-value and parent if f-value is less)
                bool no_change = false;
                for (auto p = open_list.begin(); p != open_list.end(); p++)
                {
                    if (possible_geo == (*p).second.second)
                    {
                        if (f < (*p).first)
                        {
                            open_list.erase(p);
                            break;
                        }
                        else
                        {
                            no_change = true;
                            break;
                        }
                    }
                }
                if (!no_change)
                {
                open_list.insert(make_pair(f,make_pair((*p).start,(*p).end)));
                }
            }
            
        }
    }
    /*
     First: create an "open list" to store potential values (make a list of pairs of geocoords and f-values),
     Second: create a pair for the starting point, and assign f-value of 0; make this equal to "previous geocoord"
     third: loop while the open list is empty
     fourth: pop off the first item on the list (highest priority), make this one the parent point now (keep its f-value as well), add this to the expandable hash map,check if this is the destination
     fifth: find all valid street segments from that point and add to the priority queue with their f-value (probably need a for loop), as well as their parent geocoord; if already in priority queue, then change its f-value and parent point (assuming f-value is less)
     sixth: pair of geocoords
     
     reconstruct the route using the map points
     
     */
    
    return DELIVERY_SUCCESS; // route could not be found
}

//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
