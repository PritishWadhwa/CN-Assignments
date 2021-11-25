#include "node.h"
#include <iostream>

using namespace std;

int phase = 0;

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
    bool tablesChanged = false;
    do
    {
        vector<struct routingtbl> routingTable;
        tablesChanged = false;
        // The following loop is to get the routing table of each node and send own routing table with others as it has not converged yet
        for (auto i : nd)
        {
            routingTable.push_back(i->getTable());
            i->sendMsg();
        }
        // The following loop is to check if the routing table has converged or not
        for (int i = 0; i < routingTable.size(); i++)
        {
            if (routingTable[i].tbl.size() != nd[i]->getTable().tbl.size())
            {
                tablesChanged = true;
                break;
            }
            for (int j = 0; j < nd[i]->getTable().tbl.size(); j++)
            {
                if (routingTable[i].tbl[j].nexthop != nd[i]->getTable().tbl[j].nexthop)
                {
                    tablesChanged = true;
                    break;
                }
                else
                {
                    if (routingTable[i].tbl[j].ip_interface != nd[i]->getTable().tbl[j].ip_interface)
                    {
                        tablesChanged = true;
                        break;
                    }
                }
            }
        }
    } while (tablesChanged);
    /*Print routing table entries after routing algo converges */
    printRT(nd);
}

void routingAlgo2(vector<RoutingNode *> nd)
{
    // Code to implement the routing algorithm
    int num = nd.size();
    bool nc = false;
    vector<struct routingtbl> rtb;
    int num2 = 0;
    while (true)
    {
        nc = true; //true if tables are not changed in an iteration yet
        rtb.clear();
        for (int j = 0; j < num; j++) //takes a copy of the tables before sending messages in this iteration starts
        {
            rtb.push_back(nd[j]->getTable());
        }
        for (int i = 0; i < num; i++) //sends messages to all other nodes
        {
            nd[i]->sendMsg();
        }
        for (int k = 0; k < num; k++) // checks for conv
        {
            if (rtb[k].tbl.size() != (nd[k]->getTable()).tbl.size())
            {
                nc = false;
                break;
            }
            for (int k3 = 0; k3 < (nd[k]->getTable()).tbl.size(); k3++) //compares the current and previous table
            {
                if (rtb[k].tbl[k3].nexthop == (nd[k]->getTable()).tbl[k3].nexthop)
                {
                    if (rtb[k].tbl[k3].ip_interface == (nd[k]->getTable()).tbl[k3].ip_interface)
                        continue;
                    else
                    {
                        nc = false;
                        break;
                    }
                }
                else
                {
                    nc = false;
                    break;
                }
            }
        }
        if (nc)
            break;
        printRT(nd);
        cout << "\n";
    }
    printRT(nd);
    phase = 1;
}

// void RoutingNode::recvMsg(RouteMsg *msg)
// {
//     int lim = msg->mytbl->tbl.size();
//     int lim2 = mytbl.tbl.size();
//     for (int i = 0; i < lim; i++)
//     {
//         // cout << msg->mytbl->tbl.size() << "hi" << endl;
//         RoutingEntry currEntry = msg->mytbl->tbl[i];
//         currEntry.nexthop = msg->from;
//         currEntry.ip_interface = msg->recvip;
//         currEntry.cost += 1;
//         bool alreadyPresent = false;
//         for (int j = 0; j < lim2; j++)
//         {
//             if (msg->mytbl->tbl[i].dstip == mytbl.tbl[j].dstip)
//             {
//                 alreadyPresent = true;
//                 if (msg->mytbl->tbl[i].cost + 1 < mytbl.tbl[j].cost)
//                 {
//                     if (!isMyInterface(msg->mytbl->tbl[i].nexthop))
//                     {
//                         mytbl.tbl[j].nexthop = msg->from;
//                         mytbl.tbl[j].cost = msg->mytbl->tbl[j].cost + 1;
//                         mytbl.tbl[j].ip_interface = msg->recvip;
//                     }
//                     break;
//                 }
//             }
//         }
//         if (!alreadyPresent)
//         {
//             mytbl.tbl.push_back(currEntry);
//             // break;
//         }
//     }
// }

void RoutingNode::recvMsg(RouteMsg *msg)
{
    //your code here

    int cnt = msg->mytbl->tbl.size();
    int index = 0;
    while (index < cnt)
    {
        int j = 0;
        bool found = false;
        for (j = 0; j < mytbl.tbl.size(); j++)
        {
            if (msg->mytbl->tbl[index].dstip == mytbl.tbl[j].dstip)
            {
                found = true;
                break;
            }
        }

        if (found)
        {
            if (phase == 0)
                mytbl.tbl[j].cost = min(mytbl.tbl[j].cost, msg->mytbl->tbl[index].share_cost + 1);
            else if ((msg->from.compare("10.0.1.23") == 0 || msg->from.compare("10.0.1.3") == 0)) //if message is being received though a broken link, then it is discared, here B to C is broken
                return;
            else if (!isMyInterface(mytbl.tbl[j].dstip)) //update cost if the the destination is not me[node presently]
            {
                mytbl.tbl[j].cost = min(16, msg->mytbl->tbl[index].share_cost + 1); //setting the maximimum cost limit to 16, that is infinity
                if (mytbl.tbl[j].cost < 16)                                         //if cost has been updated then the next hop for the link o be updated too
                    mytbl.tbl[j].nexthop = msg->mytbl->tbl[index].ip_interface;
            }
        }
        else
        {
            RoutingEntry n;
            n.ip_interface = msg->recvip;
            n.nexthop = msg->from;
            n.dstip = msg->mytbl->tbl[index].dstip;
            if (msg->from == n.dstip)
                n.cost = 1;
            else
                n.cost = msg->mytbl->tbl[index].cost + 1;
            mytbl.tbl.push_back(n);
        }
        index += 1;
    }
}

void RoutingNode::recvMsg1(RouteMsg *msg)
{
    //your code here
    // Function to recieve incoming routing table for each node in distance vector algorithm
    int n = mytbl.tbl.size();
    int k = msg->mytbl->tbl.size();
    bool efound = false;
    int i = 0, j = 0;
    while (j < k)
    {
        mytbl.tbl.push_back(msg->mytbl->tbl[j]);
        mytbl.tbl[mytbl.tbl.size() - 1].cost++;
        mytbl.tbl[mytbl.tbl.size() - 1].nexthop = msg->from;
        mytbl.tbl[mytbl.tbl.size() - 1].ip_interface = msg->recvip;
        efound = false; //if entry for this ip is already found then it is true
        for (i = 0; i < n; i++)
        {
            if (((msg->mytbl->tbl[j]).dstip) == (mytbl.tbl[i].dstip))
            {
                efound = true;
                if ((msg->mytbl->tbl[j].cost + 1) < mytbl.tbl[i].cost) //if a lower cost path is available it will be updated
                {
                    if (isMyInterface(msg->mytbl->tbl[j].nexthop)) //if the path is through me i will not consider it
                    {
                        break;
                    }
                    else if ((msg->from.compare("10.0.1.23") == 0 || msg->from.compare("10.0.1.3") == 0)) //if message is being received though a broken link, then it is discared, here B to C is broken
                        return;
                    else if (!isMyInterface(mytbl.tbl[j].dstip)) //update cost if the the destination is not me[node presently]
                    {
                        mytbl.tbl[j].cost = min(16, msg->mytbl->tbl[i].cost + 1); //setting the maximimum cost limit to 16, that is infinity
                        if (mytbl.tbl[j].cost < 16)                               //if cost has been updated then the next hop for the link o be updated too
                            mytbl.tbl[j].nexthop = msg->mytbl->tbl[i].ip_interface;
                    }
                    else
                    {
                        mytbl.tbl[i].nexthop = msg->from;
                        mytbl.tbl[i].ip_interface = msg->recvip;
                        mytbl.tbl[i].cost = msg->mytbl->tbl[j].cost + 1;
                        break;
                    }
                }
            }
        }

        if (efound) // removes extra entry if required
        {
            mytbl.tbl.pop_back();
        }

        j++;
    }
}

void RoutingNode::recvMsg2(RouteMsg *msg)
{
    //your code here
    // Function to recieve incoming routing table for each node in distance vector algorithm
    int n = mytbl.tbl.size();
    int k = msg->mytbl->tbl.size();
    bool efound = false;
    int i = 0, j = 0;
    while (j < k)
    {
        mytbl.tbl.push_back(msg->mytbl->tbl[j]);
        mytbl.tbl[mytbl.tbl.size() - 1].cost++;
        mytbl.tbl[mytbl.tbl.size() - 1].nexthop = msg->from;
        mytbl.tbl[mytbl.tbl.size() - 1].ip_interface = msg->recvip;
        efound = false; //if entry for this ip is already found then it is true
        for (i = 0; i < n; i++)
        {
            if (((msg->mytbl->tbl[j]).dstip) == (mytbl.tbl[i].dstip))
            {
                efound = true;
                if ((msg->mytbl->tbl[j].cost + 1) < mytbl.tbl[i].cost) //if a lower cost path is available it will be updated
                {
                    if (isMyInterface(msg->mytbl->tbl[j].nexthop)) //if the path is through me i will not consider it
                    {
                        break;
                    }
                    else if ((msg->from.compare("10.0.1.23") == 0 || msg->from.compare("10.0.1.3") == 0)) //if message is being received though a broken link, then it is discared, here B to C is broken
                        return;
                    else if (!isMyInterface(mytbl.tbl[j].dstip)) //update cost if the the destination is not me[node presently]
                    {
                        mytbl.tbl[j].cost = min(16, msg->mytbl->tbl[i].cost + 1); //setting the maximimum cost limit to 16, that is infinity
                        if (mytbl.tbl[j].cost < 16)                               //if cost has been updated then the next hop for the link o be updated too
                            mytbl.tbl[j].nexthop = msg->mytbl->tbl[i].ip_interface;
                    }
                    else
                    {
                        mytbl.tbl[i].nexthop = msg->from;
                        mytbl.tbl[i].ip_interface = msg->recvip;
                        mytbl.tbl[i].cost = msg->mytbl->tbl[j].cost + 1;
                        break;
                    }
                }
            }
        }

        if (efound) // removes extra entry if required
        {
            mytbl.tbl.pop_back();
        }

        j++;
    }
}