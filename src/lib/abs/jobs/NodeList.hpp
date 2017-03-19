#ifndef NodeListH
#define NodeListH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include <ABSMainHeader.hpp>
#include <Node.hpp>
#include <Queue.hpp>
#include <list>
#include <set>
#include <map>
#include <string>

// -----------------------------------------------------------------------------

/// group of nodes

class ABS_PACKAGE CNodeGroup : public std::set<CNodePtr> {
public:
    std::string             GroupName;
    std::set<std::string>   CommonProps;
    std::list<CNodePtr>     SortedNodes;

    /// prepare sorted list of nodes
    void MakeSortedNodeList(void);

    /// generate common properties
    void GenerateCommonProperties(void);

    /// compare node names
    static bool CompareNamesB(const CNodePtr& left,const CNodePtr& right);
};

typedef boost::shared_ptr<CNodeGroup>  CNodeGroupPtr;

// -----------------------------------------------------------------------------

/// list of all nodes

class ABS_PACKAGE CNodeList : public std::list<CNodePtr> {
public:
// constructor -----------------------------------------------------------------
    CNodeList(void);

// information methods ---------------------------------------------------------
    /// print info about all queues
    void PrintInfos(std::ostream& sout);

    /// print overal statistics
    void PrintStatistics(std::ostream& sout);

    /// print node names
    void PrintNames(std::ostream& sout);

    /// print defined node group names
    void PrintNodeGroupNames(std::ostream& sout);

    /// find node
    const CNodePtr FindNode(const CSmallString& name);

    /// find node
    const CNodePtr FindNode(const CSmallString& name,const CQueuePtr& p_queue);

    /// generate node groups
    void PrepareNodeGroups(void);

    /// put all nodes in one gorup
    void PrepareSingleNodeGroup(void);

    /// keep only those node groups that are in the list
    void KeepNodeGroups(const CSmallString& groups);

    /// finalize node groups - prepare ReasonableNodes and UniqueProps
    void FinalizeNodeGroups(void);

    /// keep only those nodes that has at least one of properties
    void KeepNodesThatHaveProperty(const std::vector<std::string>& props);

    /// keep nodes by mask
    void KeepNodesByMask(struct SExpression* p_mask);

    /// remove nodes without properties
    void RemoveNodesWithoutProps(void);

    /// remove down
    void RemoveDownNodes(void);

// section of private data -----------------------------------------------------
private:
    std::list<CNodeGroupPtr>            NodeGroups;
    std::set<CNodePtr>                  ReasonableNodes;
    std::set<std::string>               UniqueProps;
    std::map<std::string,std::string>   HWDatabase;

    // helper methods
    bool IsNodeSelected(CNodePtr p_node,struct SExpression* p_expr);
    bool IsNodeSelected(CNodePtr p_node,struct SSelection* p_sel);
};

// -----------------------------------------------------------------------------

extern CNodeList NodeList;

// -----------------------------------------------------------------------------

#endif
