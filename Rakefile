
$:.unshift File.dirname(__FILE__) + "/tasks"
require 'docker'

task :default => "release:test"

@conan_opts = {  build_parallel: 'False' }
@conan_settings = {}
@conan_scopes = { build_tests: 'True' }
@conan_build = "outdated"
load 'config.rb' if FileTest.readable? 'config.rb'


## TODO. Add task to check for git submodules...

buildTypes =  ['Debug','Release']
buildTypes.each { |build_type|
  namespace build_type.downcase.to_sym do
    build_dir = ENV['BUILD_DIR'] || "build-#{build_type}"

    @conan_settings[:build_type] = build_type
    conan_opts = @conan_opts.each_pair.map { |key,val| "-o %s=%s" % [key,val] } +
                @conan_settings.each_pair.map { |key,val| "-s %s=%s" % [key,val] } +
                @conan_scopes.each_pair.map { |key,val| "--scope %s=%s" % [key,val] }

    task :build do
      FileUtils::mkdir build_dir unless FileTest::directory? build_dir
      sh "conan source ."
      chdir build_dir do
        sh "conan install %s .. --build=%s" % [conan_opts.join(' '), @conan_build]
        sh "conan build .."
      end
    end

    task :test => :build do
      #
    end
  end
}


#DockerTasks.new

namespace :docker do
  namespace :deploy do
    task :base do
        sh "docker build --tag camhd_motion_analysis_base:latest --tag camhd_motion_analysis_base:#{`git rev-parse --short HEAD`.chomp} docker/base"
    end

    task :build => :base do
      chdir "docker/deploy" do
        sh "docker build --tag camhd_motion_analysis:latest --tag camhd_motion_analysis:#{`git rev-parse --short HEAD`.chomp} docker-deploy"
      end
    end

    task :test do
      sh "docker run camhd_motion_analysis:latest --help"
    end

    task :push_gcr => :test do
      registry_url = "us.gcr.io/camhd-image-statistics/camhd_motion_analysis"
      sh "docker tag camhd_motion_analysis:latest #{registry_url}"
      sh "gcloud docker -- push #{registry_url}"
    end
  end
end


namespace :rq do
  task :worker do
    ## Run docker at top level of repo, but reference docker/rq_worker/Dockerfile
    ## This lets us ADD the whole repo
    sh "docker build --tag camhd_motion_analysis_rq_worker:latest --tag camhd_motion_analysis_rq_worker:#{`git rev-parse --short HEAD`.chomp} --file docker/rq_worker/Dockerfile ."
  end
end


task :stitch do
  sh "build-Debug/bin/stitch --display --regions /home/aaron/workspace/camhd_analysis/CamHD_motion_metadata/RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z_optical_flow_regions.json"
end


task :process_short do
  sh "build-Debug/bin/frame_stats  --display -o CAMHDA301-20160101T000000Z_short.json --start-at 5000 --stop-at 5050 --stride 10 /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov
"
end

task :process_gpu do
  sh "build-Debug/bin/frame_stats --gpu -j 1 -o CAMHDA301-20160101T000000Z_gpu.json --start-at 5000 --stop-at 6000 --stride 10 /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov
"
end

task :process do
  sh "build-Release/bin/frame_stats  -o CAMHDA301-20160101T000000Z.json --stride 10 /RS03ASHS/PN03B/06-CAMHDA301/2016/01/01/CAMHDA301-20160101T000000Z.mov
"
end



namespace :timelapse do
  task :get do
    sh "rm -f timelapse/*"
    sh "time build-Debug/bin/timelapse"
  end

  task :encode do
    sh "cat timelapse/*.png | ffmpeg -y -framerate 2 -f image2pipe -i -  -pix_fmt yuv420p timelapse.mov"
  end
end

namespace :sequence do
  task :get do
    sh "rm -f sequence/*"
    sh "time build-Debug/bin/sequence"
  end

  task :encode do
    sh "cat sequence/*.png | ffmpeg -y -framerate 3 -f image2pipe -i - -pix_fmt yuv420p sequence.mov"
  end
end
