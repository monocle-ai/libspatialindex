/******************************************************************************
 * Project:  libspatialindex - A C++ library for spatial indexing
 * Author:   Marios Hadjieleftheriou, mhadji@gmail.com
 ******************************************************************************
 * Copyright (c) 2002, Marios Hadjieleftheriou
 *
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
******************************************************************************/

#pragma once

#include "Statistics.h"
#include "Node.h"
#include "PointerPoolNode.h"
#include <memory>

namespace SpatialIndex
{
	namespace MVRTree
	{
		class MVRTree : public ISpatialIndex
		{
			class NNEntry;
			class RootEntry;

		public:
			MVRTree(IStorageManager&, Tools::PropertySet&);
				// String                   Value     Description
				// ----------------------------------------------
				// IndexIndentifier         VT_LONG   If specified an existing index will be openened from the supplied
				//                                    storage manager with the given index id. Behaviour is unspecified
				//                                    if the index id or the storage manager are incorrect.
				// Dimension                VT_ULONG  Dimensionality of the data that will be inserted.
				// IndexCapacity            VT_ULONG  The index node capacity. Default is 100.
				// LeafCapactiy             VT_ULONG  The leaf node capacity. Default is 100.
				// FillFactor               VT_DOUBLE The fill factor. Default is 70%
				// TreeVariant              VT_LONG   Can be one of Linear, Quadratic or Rstar. Default is Rstar
				// NearMinimumOverlapFactor VT_ULONG  Default is 32.
				// SplitDistributionFactor  VT_DOUBLE Default is 0.4
				// ReinsertFactor           VT_DOUBLE Default is 0.3
				// EnsureTightMBRs          VT_BOOL   Default is true
				// IndexPoolCapacity        VT_LONG   Default is 100
				// LeafPoolCapacity         VT_LONG   Default is 100
				// RegionPoolCapacity       VT_LONG   Default is 1000
				// PointPoolCapacity        VT_LONG   Default is 500
				// StrongVersionOverflow    VT_DOUBLE Default is 0.8
				// VersionUnderflow         VT_DOUBLE Default is 0.3

			~MVRTree() override;

			//
			// ISpatialIndex interface
			//
			void insertData(uint32_t len, const byte* pData, const IShape& shape, id_type id) override;
			bool deleteData(const IShape& shape, id_type id) override;
			void containsWhatQuery(const IShape& query, IVisitor& v) override;
			void intersectsWithQuery(const IShape& query, IVisitor& v) override;
			void pointLocationQuery(const Point& query, IVisitor& v) override;
			void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v, INearestNeighborComparator&) override;
			void nearestNeighborQuery(uint32_t k, const IShape& query, IVisitor& v) override;
			void selfJoinQuery(const IShape& s, IVisitor& v) override;
			void queryStrategy(IQueryStrategy& qs) override;
			void getIndexProperties(Tools::PropertySet& out) const override;
			void addCommand(ICommand* pCommand, CommandType ct) override;
			bool isIndexValid() override;
			void getStatistics(IStatistics** out) const override;
			void flush() override;

		private:
			void initNew(Tools::PropertySet&);
			void initOld(Tools::PropertySet& ps);
			void storeHeader();
			void loadHeader();

			void insertData_impl(uint32_t dataLength, byte* pData, TimeRegion& mbr, id_type id);
			void insertData_impl(uint32_t dataLength, byte* pData, TimeRegion& mbr, id_type id, uint32_t level);
			bool deleteData_impl(const TimeRegion& mbr, id_type id);

			id_type writeNode(Node*);
			NodePtr readNode(id_type id);
			void deleteNode(Node* n);

			void rangeQuery(RangeQueryType type, const IShape& query, IVisitor& v);

			void findRootIdentifiers(const Tools::IInterval& ti, std::vector<id_type>& ids);
			std::string printRootInfo() const;

			IStorageManager* m_pStorageManager;

			std::vector<RootEntry> m_roots;
			id_type m_headerID;

			MVRTreeVariant m_treeVariant;

			double m_fillFactor;

			uint32_t m_indexCapacity;

			uint32_t m_leafCapacity;

			uint32_t m_nearMinimumOverlapFactor;
				// The R*-Tree 'p' constant, for calculating nearly minimum overlap cost.
				// [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
				// for Points and Rectangles, Section 4.1]

			double m_splitDistributionFactor;
				// The R*-Tree 'm' constant, for calculating spliting distributions.
				// [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
				// for Points and Rectangles, Section 4.2]

			double m_reinsertFactor;
				// The R*-Tree 'p' constant, for removing entries at reinserts.
				// [Beckmann, Kriegel, Schneider, Seeger 'The R*-tree: An efficient and Robust Access Method
				//  for Points and Rectangles, Section 4.3]

			double m_strongVersionOverflow;
			//double m_strongVersionUnderflow;
			double m_versionUnderflow;

			uint32_t m_dimension;

			TimeRegion m_infiniteRegion;

			SpatialIndex::MVRTree::Statistics m_stats;

			bool m_bTightMBRs;

			bool m_bHasVersionCopied;

			double m_currentTime;

			Tools::PointerPool<Point> m_pointPool;
			Tools::PointerPool<TimeRegion> m_regionPool;
			Tools::PointerPool<Node> m_indexPool;
			Tools::PointerPool<Node> m_leafPool;

			std::vector<std::shared_ptr<ICommand> > m_writeNodeCommands;
			std::vector<std::shared_ptr<ICommand> > m_readNodeCommands;
			std::vector<std::shared_ptr<ICommand> > m_deleteNodeCommands;

			class RootEntry
			{
			public:
				RootEntry() {}
				RootEntry(id_type id, double s, double e) : m_id(id), m_startTime(s), m_endTime(e) {}

				id_type m_id;
				double m_startTime;
				double m_endTime;
			}; // RootEntry

			class NNEntry
			{
			public:
				id_type m_id;
				IEntry* m_pEntry;
				double m_minDist;

				NNEntry(id_type id, IEntry* e, double f) : m_id(id), m_pEntry(e), m_minDist(f) {}
				~NNEntry() {}

				struct greater : public std::binary_function<NNEntry*, NNEntry*, bool>
				{
					bool operator()(const NNEntry* __x, const NNEntry* __y) const { return __x->m_minDist > __y->m_minDist; }
				};
			}; // NNEntry

			class NNComparator : public INearestNeighborComparator
			{
			public:
				double getMinimumDistance(const IShape& query, const IShape& entry) override
				{
					return query.getMinimumDistance(entry);
				}
				double getMinimumDistance(const IShape& query, const IData& data) override
				{
					IShape* pR;
					data.getShape(&pR);
					double ret = query.getMinimumDistance(*pR);
					delete pR;
					return ret;
				}
			}; // NNComparator

			class ValidateEntry
			{
			public:
				ValidateEntry(id_type pid, TimeRegion& r, NodePtr& pNode) : m_parentID(pid), m_parentMBR(r), m_pNode(pNode), m_bIsDead(false) {}

				id_type m_parentID;
				TimeRegion m_parentMBR;
				NodePtr m_pNode;
				bool m_bIsDead;
			}; // ValidateEntry

			friend class Node;
			friend class Leaf;
			friend class Index;

			friend std::ostream& operator<<(std::ostream& os, const MVRTree& t);
		}; // MVRTree

		std::ostream& operator<<(std::ostream& os, const MVRTree& t);
	}
}

