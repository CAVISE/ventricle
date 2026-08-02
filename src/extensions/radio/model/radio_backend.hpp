#pragma once

#include "src/core/defs.hpp"
#include "src/core/status.hpp"
#include "src/core/types.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <vector>

namespace NVentricle::NRadio {
	enum class EAccessTechnology : std::uint8_t {
		ItsG5,
		NrV2x,
	};

	struct TPacket {
		std::vector<std::byte> Payload;
	};

	struct TTransmitRequest {
		NCore::TStationId Source;
		TPacket Packet;
		std::uint8_t TrafficClass = 0;
	};

	struct TReceiveIndication {
		NCore::TStationId Destination;
		NCore::TStationId Source;
		TPacket Packet;
		NCore::TSimTime ReceptionTime;
		double SignalDbm = 0.0;
	};

	using TReceiveHandler = std::function<void(TReceiveIndication)>;

	class IRadioBackend {
	public:
		virtual ~IRadioBackend() = default;

		virtual EAccessTechnology Technology() const = 0;
		VCLE_NODISCARD virtual NCore::TStatus AddStation(NCore::TStationId stationId) = 0;
		VCLE_NODISCARD virtual NCore::TStatus RemoveStation(NCore::TStationId stationId) = 0;
		VCLE_NODISCARD virtual NCore::TStatus Transmit(TTransmitRequest request) = 0;
		virtual void SetReceiveHandler(TReceiveHandler handler) = 0;
	};
} // namespace NVentricle::NRadio
