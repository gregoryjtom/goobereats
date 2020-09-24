#include "provided.h"
#include <vector>
#include <string>

using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
    
private:
    const StreetMap* m_stmap;
    string findDirFromAngle(const double angle) const;
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm):m_stmap(sm)
{
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    DeliveryOptimizer optimize(m_stmap);
    double oldCrow,newCrow;
    vector<DeliveryRequest> copy = deliveries;
    optimize.optimizeDeliveryOrder(depot, copy, oldCrow, newCrow);
    PointToPointRouter router(m_stmap);
    totalDistanceTravelled = 0;
    
    // calculate from depot to first location
    GeoCoord start = depot;
    GeoCoord end = copy[0].location;
    if (start == end)
    {
        DeliveryCommand next_command;
        next_command.initAsDeliverCommand(copy[0].item);
        commands.push_back(next_command);
    }
    else
    {
        double dist = 0;
        list<StreetSegment> routeToTake;
        DeliveryResult test = router.generatePointToPointRoute(start, end, routeToTake, dist);
        if (test == NO_ROUTE)
            return NO_ROUTE;
        if (test == BAD_COORD)
            return BAD_COORD;
        totalDistanceTravelled += dist;
        
        double seg_dist = 0;
        StreetSegment currentStreet = *routeToTake.begin();
        for (auto p = routeToTake.begin(); p != routeToTake.end(); p++)
        {
            seg_dist += distanceEarthMiles((*p).start,(*p).end);
            if ((*p).name != currentStreet.name)
            {
                DeliveryCommand next_command;
                next_command.initAsProceedCommand(findDirFromAngle(angleOfLine(currentStreet)), currentStreet.name, seg_dist);
                commands.push_back(next_command);
                seg_dist = 0;
                
                // will only emit turn command if between the correct angles
                // otherwise will postpone the proceed command to the next time the street changes
                double angle_diff = angleBetween2Lines(currentStreet,(*p));
                if (angle_diff > 1.0 && angle_diff < 359.0)
                {
                    DeliveryCommand turn;
                    if (angle_diff < 180)
                        turn.initAsTurnCommand("left", (*p).name);
                    else
                        turn.initAsTurnCommand("right", (*p).name);
                    commands.push_back(turn);
                }
                currentStreet = (*p); // this is the new street we're on
            }
            if ((*p).end == end)
            {
                DeliveryCommand next_command;
                next_command.initAsProceedCommand(findDirFromAngle(angleOfLine(currentStreet)),currentStreet.name,seg_dist);
                seg_dist = 0;
                commands.push_back(next_command);
                DeliveryCommand final_command;
                final_command.initAsDeliverCommand(copy[0].item);
                commands.push_back(final_command);
            }
        }
    }
   
    
    // calculate from N location to N+1 location
    for (int i = 0; i != copy.size(); i++)
    {
        double dist = 0;
        list<StreetSegment> routeToTake;
        if (i != copy.size()-1)
        {
            GeoCoord start = copy[i].location;
            GeoCoord end = copy[i+1].location;
            DeliveryResult test = router.generatePointToPointRoute(start, end, routeToTake, dist);
            if (test == NO_ROUTE)
                return NO_ROUTE;
            if (test == BAD_COORD)
                return BAD_COORD;
            if (start == end)
            {
                DeliveryCommand next_command;
                next_command.initAsDeliverCommand(copy[i+1].item);
                commands.push_back(next_command);
            }
            else
            {
                double seg_dist = 0;
                StreetSegment currentStreet = *routeToTake.begin();
                for (auto p = routeToTake.begin(); p != routeToTake.end(); p++)
                {
                    seg_dist += distanceEarthMiles((*p).start,(*p).end);
                    if ((*p).name != currentStreet.name)
                    {
                        DeliveryCommand next_command;
                        next_command.initAsProceedCommand(findDirFromAngle(angleOfLine(currentStreet)), currentStreet.name, seg_dist);
                        commands.push_back(next_command);
                        seg_dist = 0;
                        
                        // will only emit turn command if between the correct angles
                        // otherwise will postpone the proceed command to the next time the street changes
                        double angle_diff = angleBetween2Lines(currentStreet,(*p));
                        if (angle_diff > 1.0 && angle_diff < 359.0)
                        {
                            DeliveryCommand turn;
                            if (angle_diff < 180)
                                turn.initAsTurnCommand("left", (*p).name);
                            else
                                turn.initAsTurnCommand("right", (*p).name);
                            commands.push_back(turn);
                        }
                        currentStreet = (*p); // this is the new street we're on
                    }
                    if ((*p).end == end)
                    {
                        DeliveryCommand next_command;
                        next_command.initAsProceedCommand(findDirFromAngle(angleOfLine(currentStreet)),currentStreet.name,seg_dist);
                        seg_dist = 0;
                        commands.push_back(next_command);
                        DeliveryCommand final_command;
                        final_command.initAsDeliverCommand(copy[i+1].item);
                        commands.push_back(final_command);
                    }
                }
            }
        }
        else
        {
            GeoCoord start = copy[i].location;
            GeoCoord end = depot;
            DeliveryResult test = router.generatePointToPointRoute(start, end, routeToTake, dist);
            if (test == NO_ROUTE)
                return NO_ROUTE;
            if (test == BAD_COORD)
                return BAD_COORD;
            if (start != end)
            {
                double seg_dist = 0;
                StreetSegment currentStreet = *routeToTake.begin();
                for (auto p = routeToTake.begin(); p != routeToTake.end(); p++)
                {
                    seg_dist += distanceEarthMiles((*p).start,(*p).end);
                    if ((*p).name != currentStreet.name)
                    {
                        DeliveryCommand next_command;
                        next_command.initAsProceedCommand(findDirFromAngle(angleOfLine(currentStreet)), currentStreet.name, seg_dist);
                        commands.push_back(next_command);
                        seg_dist = 0;
                        
                        // will only emit turn command if between the correct angles
                        // otherwise will postpone the proceed command to the next time the street changes
                        double angle_diff = angleBetween2Lines(currentStreet,(*p));
                        if (angle_diff > 1.0 && angle_diff < 359.0)
                        {
                            DeliveryCommand turn;
                            if (angle_diff < 180)
                                turn.initAsTurnCommand("left", (*p).name);
                            else
                                turn.initAsTurnCommand("right", (*p).name);
                            commands.push_back(turn);
                        }
                        currentStreet = (*p); // this is the new street we're on
                    }
                    if ((*p).end == end)
                    {
                        DeliveryCommand final_command;
                        final_command.initAsProceedCommand(findDirFromAngle(angleOfLine(currentStreet)),currentStreet.name,seg_dist);
                        seg_dist = 0;
                        commands.push_back(final_command);
                    }
                }
            }
        }
        totalDistanceTravelled += dist;
    }
    return DELIVERY_SUCCESS;  
}
                                                          
string DeliveryPlannerImpl::findDirFromAngle(const double angle) const
{
    if (angle>= 0 && angle<22.5)
        return "east";
    else if (angle < 67.5)
        return "northeast";
    else if (angle < 112.5)
        return "north";
    else if (angle < 157.5)
        return "northwest";
    else if (angle < 202.5)
        return "west";
    else if (angle < 247.5)
        return "southwest";
    else if (angle < 292.5)
        return "south";
    else if (angle < 337.5)
        return "southeast";
    else
        return "east";
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
