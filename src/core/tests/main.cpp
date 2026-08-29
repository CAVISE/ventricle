#include <absl/log/initialize.h>

#include <ns3/simulator.h>
#include <ns3/test.h>

int main(int argc, char** argv) {
	absl::InitializeLog();
	const auto result = ns3::TestRunner::Run(argc, argv);

	// Clear tracked Time objects before ns-3's static mutex is destroyed.
	ns3::Simulator::Run();
	ns3::Simulator::Destroy();

	return result;
}
