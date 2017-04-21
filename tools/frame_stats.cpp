#include <string>
#include <fstream>
#include <sstream>

#include <g3log/g3log.hpp>
#include <g3log/logworker.hpp>

#include <curlpp/cURLpp.hpp>

#include <tclap/CmdLine.h>

#include "camhd_client.h"
#include "interval.h"

#include "movie_workers/frame_processor.h"
#include "movie_workers/frame_mean.h"
#include "movie_workers/frame_statistics.h"
#include "movie_workers/approx_derivative.h"

#include "json.hpp"
using json = nlohmann::json;

using namespace std;
//using namespace cv;

using namespace CamHDMotionTracking;

const fs::path DefaultCacheURL( "https://camhd-app-dev.appspot.com/v1/org/oceanobservatories/rawdata/files");


bool doStop = false;

void catchSignal(int signo) {
	switch( signo ) {
		case SIGINT:
				doStop = true;
				return;
	}
}



class FrameStatsConfig {
public:
	FrameStatsConfig()
	{}

		bool parseArgs( int argc, char **argv )
		{
			try {
				TCLAP::CmdLine cmd("Command description message", ' ', "0.0");

				TCLAP::UnlabeledValueArg<std::string> pathsArg("path","Path",true,"","Path",cmd);

				TCLAP::ValueArg<std::string> jsonOutArg("o", "out", "File for JSON output (leave blank for stdout)", false,jsonOut.string(), "filename", cmd );
				TCLAP::ValueArg<std::string> hostArg("","host","URL to host",false,DefaultCacheURL.string(),"url",cmd);

				TCLAP::ValueArg<int> startAtArg("","start-at","",false,startAt,"frame number",cmd);
				TCLAP::ValueArg<int> stopAtArg("","stop-at","",false,stopAt,"frame number",cmd);
				TCLAP::ValueArg<int> strideArg("","stride","Number of frames for stride",false,stride,"num of frames",cmd);

				// Parse the argv array.
				cmd.parse( argc, argv );

				// Args back to
				cacheURL = hostArg.getValue();

				startAt = startAtArg.getValue();
				startAtSet = startAtArg.isSet();

				stopAt = stopAtArg.getValue();
				stopAtSet = stopAtArg.isSet();

				stride = strideArg.getValue();

				jsonOut = jsonOutArg.getValue();
				jsonOutSet = jsonOutArg.isSet();

				path = pathsArg.getValue();

			} catch (TCLAP::ArgException &e)  {
				LOG(FATAL) << "error: " << e.error() << " for arg " << e.argId();
			}

			return true;
		}


		fs::path cacheURL;
		std::string path;
		// Set a default for testing

		fs::path jsonOut;
		bool jsonOutSet;


		int stopAt = -1;
		int startAt = -1;

		bool startAtSet, stopAtSet;

		int stride = 5000;

};

int main( int argc, char ** argv )
{
	auto worker = g3::LogWorker::createLogWorker();
  auto handle= worker->addDefaultLogger(argv[0],".");
  g3::initializeLogging(worker.get());

  // RAAI initializer for curlpp
	curlpp::Cleanup cleanup;


	if(signal(SIGINT, catchSignal ) == SIG_ERR) {
			LOG(FATAL) << "An error occurred while setting the signal handler.";
			exit(-1);
	}

	FrameStatsConfig config;
	if( !config.parseArgs( argc, argv ) ) {
		LOG(WARNING) << "Error while parsing args";
		exit(-1);
	}

	// Measure time of execution
	std::chrono::time_point<std::chrono::system_clock> start( std::chrono::system_clock::now() );

	fs::path movURL( config.cacheURL );
	movURL /= config.path;

	auto movie( CamHDClient::getMovie( movURL ) );


	// TODO.  Check for failure
	LOG(INFO) << "File has " << movie.numFrames() << " frames";

	std::vector< std::shared_ptr< FrameProcessor > > processors;
	//processor = new FrameStatistics stats(movie);
	processors.emplace_back( new ApproxDerivative(movie) );
	processors.emplace_back( new FrameStatistics(movie) );

	json jsonStats;

	const int startAt = (config.startAtSet ? std::max( 0, config.startAt ) : 0 );
	const int stopAt = (config.stopAtSet ? std::min( movie.numFrames(), config.stopAt ) : movie.numFrames() );

	for( auto i = startAt; i < stopAt && !doStop; i += config.stride ) {
		auto frame = (i==0 ? 1 : i);
		LOG(INFO) << "Processing frame " << frame;
		json j;

    j["frameNum"] = i;

		for( auto proc : processors ) {
			j[proc->jsonName()] = proc->process(frame);
		}

		if( !j.empty() ) jsonStats.push_back( j );
	}


	std::chrono::duration<double> elapsedSeconds = std::chrono::system_clock::now()-start;

	json mov;
	mov["elapsedSystemTime"] = elapsedSeconds.count();
	mov["movie"] = movie;
	mov["stats"] = jsonStats;


	if( config.jsonOutSet ) {
		ofstream f( config.jsonOut.string() );
		f << mov.dump(4) << endl;
	} else {
		cout << mov.dump(4) << endl;
	}

	exit(0);
}