#include "node.h"
#include <iostream>

using namespace std;
bool q1 = true;

void printRT(vector<RoutingNode *> nd)
{
    /*Print routing table entries*/
    for (int i = 0; i < nd.size(); i++)
    {
        nd[i]->printTable();
    }
}

void routingAlgo(vector<RoutingNode *> nd)
{
    //Your code here
    //Your code here
    bool keepRunning = true;
    bool tablesChanged;
    vector<struct routingtbl> routingTable;
    do
    {
        // vector<struct routingtbl> routingTable;
        routingTable.clear();
        tablesChanged = false;
        // The following loop is to get the routing table of each node and send own routing table with others as it has not converged yet
        for (auto i : nd)
        {
            routingTable.push_back(i->getTable());
            i->sendMsg();
        }
        // The following loop is to check if the routing table has converged or not
        for (int i = 0; i < nd.size(); i++)
        {
            if (routingTable[i].tbl.size() != (nd[i]->getTable()).tbl.size())
            {
                tablesChanged = true;
                break;
            }
            for (int j = 0; j < (nd[i]->getTable()).tbl.size(); j++)
            {
                if (routingTable[i].tbl[j].nexthop != (nd[i]->getTable()).tbl[j].nexthop)
                {
                    tablesChanged = true;
                    break;
                }
                else
                {
                    if (routingTable[i].tbl[j].ip_interface != (nd[i]->getTable()).tbl[j].ip_interface)
                    {
                        tablesChanged = true;
                        break;
                    }
                }
            }
        }
        if (!tablesChanged)
        {
            keepRunning = false;
        }
        printRT(nd);
        cout << '\n';
    } while (keepRunning);
    // variable to modify the state of ques 1
    q1 = false;
    /*Print routing table entries after routing algo converges */
    // printRT(nd);
}

void RoutingNode::recvMsg(RouteMsg *msg)
{
    //your code here
    int lim = msg->mytbl->tbl.size();
    for (int i = 0; i < lim; i++)
    {
        RoutingEntry currEntry = msg->mytbl->tbl[i];
        currEntry.nexthop = msg->from;
        currEntry.ip_interface = msg->recvip;
        if (msg->from != currEntry.dstip)
        {
            currEntry.cost = msg->mytbl->tbl[i].cost;
            currEntry.cost += 1;
        }
        else
        {
            currEntry.cost = 1;
        }
        bool alreadyPresent = false;
        for (int j = 0; j < mytbl.tbl.size(); j++)
        {
            if (msg->mytbl->tbl[i].dstip == mytbl.tbl[j].dstip)
            {
                alreadyPresent = true;
                if (q1)
                {
                    mytbl.tbl[j].cost = min(mytbl.tbl[j].cost, msg->mytbl->tbl[i].cost + 1);
                }
                else
                {
                    if (msg->from == "10.0.1.3" || msg->from == "10.0.1.23")
                    {
                        return;
                    }
                    else if (isMyInterface(mytbl.tbl[j].dstip) == false)
                    {
                        if (msg->mytbl->tbl[i].cost < 15)
                        {
                            mytbl.tbl[j].cost = msg->mytbl->tbl[i].cost + 1;
                        }
                        else
                        {
                            mytbl.tbl[j].cost = 16;
                        }
                        if (mytbl.tbl[j].cost < 16)
                        {
                            mytbl.tbl[j].nexthop = msg->mytbl->tbl[i].ip_interface;
                        }
                    }
                }
                break;
            }
        }
        if (!alreadyPresent)
        {
            mytbl.tbl.push_back(currEntry);
        }
    }
}