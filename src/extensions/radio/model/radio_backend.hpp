#pragma once

#include "src/core/defs.hpp"
#include "src/core/status.hpp"
#include "src/core/types.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace vcle::radio {
	/** Radio access technologies supported by simulator backends. */
	enum class AccessTechnology : std::uint8_t {
		ItsG5,
		NrV2x,
	};

	/** Bytes carried by a simulated radio transmission. */
	struct Packet {
		std::vector<std::byte> payload_;
	};

	/** Describes one outgoing radio packet. */
	struct TransmitRequest {
		NCore::TStationId source_;
		Packet packet_;
		std::uint8_t trafficClass_ = 0;
	};

	/** Describes one packet delivered by a radio backend. */
	struct ReceiveIndication {
		NCore::TStationId destination_;
		NCore::TStationId source_;
		Packet packet_;
		NCore::TSimTime receptionTime_;
		double signalDbm_ = 0.0;
	};

	/** Callback invoked for received radio packets. */
	using ReceiveHandler = std::function<void(ReceiveIndication)>;

	/** Abstracts a radio technology implementation from its users. */
	class IRadioBackend {
	public:
		virtual ~IRadioBackend() = default;

		/** Return the technology implemented by this backend. */
		virtual AccessTechnology technology() const = 0;
		/** Register @p stationId with the backend. */
		VCLE_NODISCARD virtual NCore::TStatus addStation(NCore::TStationId stationId) = 0;
		/** Remove @p stationId from the backend. */
		VCLE_NODISCARD virtual NCore::TStatus removeStation(NCore::TStationId stationId) = 0;
		/** Submit one packet for transmission. */
		VCLE_NODISCARD virtual NCore::TStatus transmit(TransmitRequest request) = 0;
		/** Replace the handler for received packets. */
		virtual void setReceiveHandler(ReceiveHandler handler) = 0;
	};
} // namespace vcle::radio
