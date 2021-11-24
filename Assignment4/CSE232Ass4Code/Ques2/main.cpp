#include "node.h"

vector<RoutingNode *> distanceVectorNodes;

void routingAlgo(vector<RoutingNode *> distanceVectorNodes);
void routingAlgo2(vector<RoutingNode *> distanceVectorNodes);

int main()
{
    int n; // number of nodes
    cin >> n;
    string name; //Node label
    distanceVectorNodes.clear();
    for (int i = 0; i < n; i++)
    {
        RoutingNode *newnode = new RoutingNode();
        cin >> name;
        newnode->setName(name);
        distanceVectorNodes.push_back(newnode);
    }
    cin >> name;
    /*
      For each node label(@name), it's own ip address, ip address of another node
      defined by @oname will be inserted in the node's own datastructure interfaces 
    */
    while (name != "EOE")
    { //End of entries
        for (int i = 0; i < distanceVectorNodes.size(); i++)
        {
            string myeth, oeth, oname;
            if (distanceVectorNodes[i]->getName() == name)
            {
                //node interface ip
                cin >> myeth;
                //ip of another node connected to myeth (nd[i])
                cin >> oeth;
                //label of the node whose ip is oeth
                cin >> oname;
                for (int j = 0; j < distanceVectorNodes.size(); j++)
                {
                    if (distanceVectorNodes[j]->getName() == oname)
                    {
                        /*
		@myeth: ip address of my (distanceVectorNodes[i]) end of connection.
		@oeth: ip address of other end of connection.
		@distanceVectorNodes[j]: pointer to the node whose one of the interface is @oeth
	      */
                        distanceVectorNodes[i]->addInterface(myeth, oeth, distanceVectorNodes[j]);
                        //Routing table initialization
                        /*
		@myeth: ip address of my (distanceVectorNodes[i]) ethernet interface.
		@0: hop count, 0 as node does not need any other hop to pass packet to itself.
		
	      */
                        distanceVectorNodes[i]->addTblEntry(myeth, 0);
                        break;
                    }
                }
            }
        }
        cin >> name;
    }

    /* The logic of the routing algorithm should go here */
    routingAlgo(distanceVectorNodes);
    /* Add the logic for periodic update (after every 1 sec) here */
    cout << "\n\n\n\n\n\n\n\n\n\n\n";
    for (int i = 0; i < distanceVectorNodes.size(); i++)
    {
        // if (distanceVectorNodes[i]->getName() == "B")
        // {
        //     auto table = distanceVectorNodes[i]->getTable().tbl;
        //     for (auto i : table)
        //     {
        //         // cout << "pritish\n";
        //         // cout << i.dstip << " " << i.nexthop << " " << i.cost << "\n";
        //         // cout << i.first << " " << i.second << endl;
        //         if (i.dstip == "10.0.1.3" && i.nexthop == "10.0.1.3")
        //         {
        //             cout << "yo1";
        //             i.cost = 16;
        //         }
        //     }
        // }
        // if (distanceVectorNodes[i]->getName() == "C")
        // {
        //     auto table = distanceVectorNodes[i]->getTable().tbl;
        //     for (auto i : table)
        //     {
        //         // cout << "pritish\n";
        //         // cout << i.dstip << " " << i.nexthop << " " << i.cost << "\n";
        //         // cout << i.first << " " << i.second << endl;
        //         if (i.dstip == "10.0.1.23" && i.nexthop == "10.0.1.23")
        //         {
        //             cout << "yo2";
        //             i.cost = 16;
        //         }
        //     }
        // }
        distanceVectorNodes[i]->updateBC();
        distanceVectorNodes[i]->printTable();
    }

    cout << "\n\n\n\n\n\n\n\n\n\n\n";

    // for (auto i : distanceVectorNodes)
    // {
    //     if (i->getName() == "B")
    //     {
    //         // cout << "yes";
    //         auto table = i->getTable().tbl;
    //         for (auto j : table)
    //         {
    //             if (j.nexthop == "10.0.1.3")
    //             {
    //                 cout << "yes1";
    //                 j.cost = 16;
    //             }
    //             //     cout << j.first << " " << j.second << endl;
    //         }
    //     }
    //     if (i->getName() == "C")
    //     {
    //         // cout << "yes";
    //         auto table = i->getTable().tbl;

    //         for (auto j : table)
    //         {
    //             if (j.nexthop == "10.0.1.23")
    //             {
    //                 cout << "yes2";
    //                 j.cost = 16;
    //             }
    //             //     cout << j.first << " " << j.second << endl;
    //         }
    //     }
    //     // if(i->getName() == 'B'){
    //     //     vector<pair<NetInterface, Node *>> interfaces = i->getInterface();
    //     //     for(auto j: interfaces)
    //     //     {
    //     //         if(j.second->getName() == 'C'){
    //     //             j.first.
    //     //         }
    //     //     }
    //     // }
    // }
    for (int i = 0; i < distanceVectorNodes.size(); i++)
    {
        distanceVectorNodes[i]->printTable();
    }
    routingAlgo2(distanceVectorNodes);
    for (int i = 0; i < distanceVectorNodes.size(); i++)
    {
        distanceVectorNodes[i]->printTable();
    }
}
