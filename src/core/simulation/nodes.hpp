#pragma once

#include "src/core/defs.hpp"

#include <functional>
#include <queue>
#include <utility>

#include <absl/container/flat_hash_set.h>

#include <ns3/fatal-error.h>
#include <ns3/node-container.h>
#include <ns3/node.h>
#include <ns3/ptr.h>

namespace vcle {

	/** Provides access to the node-management policy used by a simulation. */
	class INodeManager {
	public:
		virtual ~INodeManager() = default;
	};

	/** Manages a simulation whose active node set can change while it runs. */
	class IDynamicNodeManager : public INodeManager {
	public:
		~IDynamicNodeManager() override = default;

		/**
		 * Acquire and activate a simulation node.
		 *
		 * @return An active node, or `nullptr` when no node is available.
		 */
		VCLE_NODISCARD virtual ns3::Ptr<ns3::Node> addNode() = 0;

		/**
		 * Remove a node from the active simulation.
		 *
		 * @param node Active node previously returned by addNode().
		 *
		 * ns-3 retains every created node in its global registry until shutdown. Implementations
		 * must therefore deactivate the node and release manager-owned resources instead of
		 * destroying it.
		 */
		virtual void removeNode(ns3::Ptr<ns3::Node> node) = 0;
	};

	/** Supplies a pool of nodes for a node manager to own and recycle. */
	class INodePoolProvider {
	public:
		virtual ~INodePoolProvider() = default;

		/**
		 * Transfer a node pool to the caller.
		 *
		 * @return Pre-created nodes which are not currently active in a simulation.
		 */
		VCLE_NODISCARD virtual ns3::NodeContainer takeNodes() = 0;
	};

	/** Initializes or deinitializes one pooled node. */
	using NodeLifecycleCallback = std::function<void(ns3::Ptr<ns3::Node>)>;

	/**
	 * Activates and recycles nodes from a pool created before simulation runtime.
	 *
	 * Nodes remain owned by ns-3 for the whole simulation; removal performs caller-defined
	 * deinitialization and marks the node as reusable. Idle nodes are acquired in FIFO order.
	 */
	class DynamicNodeManager : public IDynamicNodeManager {
	public:
		/**
		 * Construct a manager for pre-created, unused nodes.
		 *
		 * Every node must be non-null and occur only once in @p nodes. @p initialize configures a
		 * node for runtime use; @p deinitialize must undo that configuration sufficiently for later
		 * reuse.
		 *
		 * @param nodes Pool of nodes created before the simulation starts.
		 * @param initialize Callback invoked before addNode() exposes a node as active.
		 * @param deinitialize Callback invoked before removeNode() makes a node reusable.
		 */
		DynamicNodeManager(const ns3::NodeContainer& nodes, NodeLifecycleCallback initialize, NodeLifecycleCallback deinitialize)
			: initialize_(std::move(initialize))
			, deinitialize_(std::move(deinitialize)) {
			absl::flat_hash_set<const ns3::Node*> uniqueNodes;
			uniqueNodes.reserve(nodes.GetN());
			activeNodes_.reserve(nodes.GetN());

			for (auto iterator = nodes.Begin(); iterator != nodes.End(); ++iterator) {
				const auto& node = *iterator;
				NS_ABORT_MSG_UNLESS(node, "dynamic node manager cannot pool a null node");
				const auto inserted = uniqueNodes.emplace(ns3::PeekPointer(node)).second;
				NS_ABORT_MSG_UNLESS(inserted, "dynamic node manager cannot pool a node twice");
				idleNodes_.push(node);
			}
		}

		/**
		 * Construct a manager using all nodes transferred by @p provider.
		 *
		 * @param provider Provider from which the manager takes its node pool.
		 * @param initialize Callback invoked before addNode() exposes a node as active.
		 * @param deinitialize Callback invoked before removeNode() makes a node reusable.
		 */
		DynamicNodeManager(INodePoolProvider& provider, NodeLifecycleCallback initialize, NodeLifecycleCallback deinitialize)
			: DynamicNodeManager(provider.takeNodes(), std::move(initialize), std::move(deinitialize)) {
		}

		DynamicNodeManager(const DynamicNodeManager&) = delete;
		DynamicNodeManager& operator=(const DynamicNodeManager&) = delete;

		/* IDynamicNodeManager implementation*/
		/** Initialize and return the first idle node, or `nullptr` if the pool is exhausted. */
		VCLE_NODISCARD ns3::Ptr<ns3::Node> addNode() override {
			if (idleNodes_.empty()) {
				return nullptr;
			}

			const auto node = idleNodes_.front();
			std::invoke(initialize_, node);
			idleNodes_.pop();
			const auto inserted = activeNodes_.emplace(ns3::PeekPointer(node)).second;
			NS_ABORT_MSG_UNLESS(inserted, "dynamic node manager activated a node twice");
			return node;
		}

		/**
		 * Deinitialize an active node and return it to the pool.
		 *
		 * @param node Active node previously returned by addNode().
		 *
		 * Passing an idle node or a node which does not belong to this manager is a fatal contract
		 * error.
		 */
		void removeNode(ns3::Ptr<ns3::Node> node) override {
			const auto iterator = activeNodes_.find(ns3::PeekPointer(node));
			NS_ABORT_MSG_UNLESS(iterator != activeNodes_.end(), "cannot return a node which is not active in this dynamic node manager");

			std::invoke(deinitialize_, node);
			activeNodes_.erase(iterator);
			idleNodes_.push(node);
		}

	private:
		std::queue<ns3::Ptr<ns3::Node>> idleNodes_;
		absl::flat_hash_set<const ns3::Node*> activeNodes_;
		NodeLifecycleCallback initialize_;
		NodeLifecycleCallback deinitialize_;
	};

} // namespace vcle
