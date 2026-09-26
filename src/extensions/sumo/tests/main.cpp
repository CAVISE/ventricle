#include <absl/log/initialize.h>

#include <ns3/core-module.h>
#include <ns3/test.h>

int main(int argc, char* argv[]) {
	absl::InitializeLog();
	ns3::TestRunner::Run(argc, argv);
	ns3::Simulator::Run();
	ns3::Simulator::Destroy();
}
