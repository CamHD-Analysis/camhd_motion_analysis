#pragma once

#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "json_converters.h"

#include "json.hpp"
using json = nlohmann::json;

#include "camhd_movie.h"
#include "camhd_client.h"

namespace CamHDMotionTracking {

  using namespace cv;
  using std::stringstream;

  struct FrameProcessor {
    FrameProcessor( const CamHDMovie &mov )
    : doDisplay( false ),
      _movie(mov)
    {;}

    virtual const string jsonName() = 0;

    // Thin convenience wrapper around process()
    json operator()( int f )
    { return asJson(f); }

    virtual json asJson( int f ) = 0;


    bool compare( int a, int b ) {

      cv::Mat imgA( getFrame(a) ), imgB( getFrame(b) );

      float meanA = cv::mean( cv::mean( imgA ) )[0];
      float meanB = cv::mean( cv::mean( imgB ) )[0];

      stringstream numA;
      numA << a << ":" << meanA;

      stringstream numB;
      numB << b << ":" << meanB;

      auto compo( composite( imgA, imgB, numA.str(), numB.str() ) );

      cv::imshow( "comparison!", compo );
      cv::waitKey(100);
      //

      const int maxDelta = 10;

      auto pct = std::abs(meanB - meanA);

      return pct < maxDelta;
    }

    cv::Mat composite( const cv::Mat &imgA, const cv::Mat &imgB,
                      const std::string &textA, const std::string &textB )
                      {
                        const cv::Point textOrigin(10,30);
                        const cv::Scalar textColor( 0,0,255);
                        //
                        cv::Mat comp( cv::Size( imgA.cols + imgB.cols, std::max(imgA.rows, imgB.rows) ), imgA.type() );

                        cv::Mat roiA( comp, cv::Rect(0,0, imgA.cols, imgA.rows ) );
                        imgA.copyTo( roiA );
                        cv::putText( roiA, textA, textOrigin, cv::FONT_HERSHEY_SIMPLEX, 1.0, textColor );

                        cv::Mat roiB( comp, cv::Rect( imgA.cols,0, imgB.cols, imgB.rows ) );
                        imgB.copyTo( roiB );


                        cv::putText( roiB, textB, textOrigin, cv::FONT_HERSHEY_SIMPLEX, 1.0, textColor );

                        return comp;

                      }

    cv::Mat getFrame( int frameNum, float scale = 0.25 ) {
      cv::Mat frame( CamHDClient::getFrame( _movie, frameNum ));
      cv::Mat reduced;

      if( frame.empty() ) return cv::Mat();

      cv::resize( frame, reduced, cv::Size(0,0), scale, scale );
      return reduced;
    }



bool doDisplay;


    CamHDMovie _movie;
  };

}
