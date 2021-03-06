#include "WgHtmlAds.h"

// C
#include <sys/stat.h>  // /* stat */ function
#include <sys/types.h> //needed for stat fucntion
#include <unistd.h>//usleep()

// C++
#include <climits> /* PATH_MAX */ //
#include <cstdlib> // /*realpath*/ funtion for cutycapt also needed PATH_MAX from limits.h
#include <iostream> // /*cout*/
#include <fstream>//ifstream
#include <iterator>     // std::distance

// A11Y libs

// own headers
#include "Engine.h" // from here we need only "fmt" stuff
#include "Timer.h"  ///* StrNow()*/
#include "configurator.h" //config
#include "json.h"

using json = nlohmann::json;

//stub show time is = (widgets update time) * STUB_TIME_MULTIPLY
#define STUB_TIME_MULTIPLY 2
//probably need to add into config or even use stub instead
#define RENDERING_ERROR_HEADER_TEXT "O_o"

WgHtmlAds::WgHtmlAds(int Ax, int Ay, WgMode Amode)
    : WgBackground(Ax, Ay, Amode) 
{
  shadows_on_ = false; // default render of shadows off
  
  int update_time;
  config->Get("ADVERT_UPDATE_TIME", update_time);

  if (update_time <= 0) {
    widget_update_time_=static_cast<unsigned int>(update_time);
    Set_widget_update_time(3000);
  } else {
    widget_update_time_=static_cast<unsigned int>(update_time);
    Set_widget_update_time(update_time * 1000); // sets update time in seconds
  }

  // advert part
  config->Get("ADVERT_DEST", advert_dest_);

  config->Get("ADVERT_JSON_PATH", adverts_json_path_);
  config->Get("ADVERT_JSON_FILE_NAME", adverts_json_name_);

  config->Get("ADVERT_STUB_PATH", advert_stub_path_);
  config->Get("ADVERT_STUB_NAME", advert_stub_name_);
  config->Get("ADVERT_STUB_TITLE", advert_stub_title_);

 // get full path to exe
  char buff[PATH_MAX];
  realpath(".", buff);               
  project_root_path_ = buff; 

  json_file_time_=this->GetJsonFileTime_();
  InitStub_();
  InitilizeAdverts_();
  current_advert_=adverts_.begin();
  current_timestamp_=std::time(0);

  std::cout << StrNow() << "\t"
            << "WgHtmlsAds widget object was created\n";

}

WgHtmlAds::~WgHtmlAds() {  }

std::time_t WgHtmlAds::GetJsonFileTime_(){
   struct stat buff;
  // check if it can touch file and read last edit time of file
  if (stat(fmt("%s/%s", adverts_json_path_.c_str(),
               adverts_json_name_.c_str()),
           &buff) == 0)
  { 
    return buff.st_mtime;
  }
  else{
    return 0;
  }
}

bool WgHtmlAds::IsNeedRenewAdverts_(){
  std::time_t ft = this->GetJsonFileTime_();
  if ((ft != 0) && (ft != json_file_time_)) {
    json_file_time_ = ft;
    return true;
  }
  else{
  	return false;
  }
}

void WgHtmlAds::CleanAdverts_(){
  //erase all from second element
  adverts_.erase(std::next(adverts_.begin(),1),adverts_.end());
  usleep(150);//get some time to destruct all previus ads
  current_advert_=adverts_.begin();
}

void WgHtmlAds::InitStub_(){
   unsigned int stub_show_time=widget_update_time_*STUB_TIME_MULTIPLY;
    adverts_.push_back(std::unique_ptr<AdvertShell>(std::move(new AdvertShell(
      RectClient.left,RectClient.bottom,
      RectClient.width,RectClient.height,
      advert_stub_path_,advert_stub_name_,
      advert_stub_title_,stub_show_time
    )
    )));
}


void WgHtmlAds::InitilizeAdverts_()
{
  //probably need in try catch statement//  
  std::ifstream i(adverts_json_path_+'/'+adverts_json_name_);
	json j;
	i >> j;

  std::string url{};
  std::string title{};
  std::time_t start_ts=0;
  std::time_t end_ts=0;
  std::time_t show_time=0;
  bool hidden=true;
  
  //valid advert means without errors
  bool is_valid_advert=true;

  if(j.size()==0){
    std::cout<<StrNow()<<
    "\tDetected what in \""<<adverts_json_name_<<"\" contains zero adverts\n";
  }

  for (json::iterator it = j.begin(); it != j.end(); ++it) {
    is_valid_advert=true;


    if(it[0]["url"].is_string()){
      url=it[0]["url"];
    }
    else{
      std::cerr<<"Do not detected \"url\" string!! at index-> "<<std::distance(j.begin(),it)<<" advert is not valid\n";
      is_valid_advert=false;
    }

    if(it[0]["title"].is_string()){
      title=it[0]["title"];
    }else{
      std::cerr<<"Did not detected \"title\" string!! at index-> "<<std::distance(j.begin(),it)<<" advert is not valid\n";
      is_valid_advert=false;
    }

    if(it[0]["start_ts"].is_number()){
      start_ts=it[0]["start_ts"].get<std::time_t>();
    }
    else{
      std::cerr<<"Did not detected \"start_ts\" number!! at index-> "<<std::distance(j.begin(),it)<<" advert is not valid\n";
      is_valid_advert=false;
    }

    if(it[0]["end_ts"].is_number()){
      end_ts=it[0]["end_ts"].get<std::time_t>();
    }
    else{
      std::cerr<<"Did not detected \"end_ts\" number!! at index-> "<<std::distance(j.begin(),it)<<" advert is not valid\n";
      is_valid_advert=false;
    }

    if(it[0]["show_time"].is_number()){
      show_time=it[0]["show_time"].get<std::time_t>();
    }
    else{
      std::cerr<<"Did not detected \"show_time\" number!! at index-> "<<std::distance(j.begin(),it)<<" advert is not valid\n";
      is_valid_advert=false;
    }

    if(it[0]["hidden"].is_boolean()){
      hidden=it[0]["hidden"];
    }
    else{
      std::cerr<<"Did not detected \"hidden\" boolean!! at index-> "<<std::distance(j.begin(),it)<<" advert is not valid\n";
      is_valid_advert=false;
    }
    //probably std::move is rudement here
    //added it because i can
    adverts_.push_back(std::unique_ptr<AdvertShell>(std::move(new AdvertShell(
      RectClient.left,RectClient.bottom,
      RectClient.width,RectClient.height,
      project_root_path_,advert_dest_,
      url,title,
      start_ts,end_ts,
      show_time,hidden,
      is_valid_advert
    ))));
  }                      
}

bool WgHtmlAds::update() {
  //detection if adverts.json was edited if so 
  if(this->IsNeedRenewAdverts_()==true){
    std::cout<< StrNow()<<"\tDetected what \""<<adverts_json_name_<<
    "\" was edited, launched update\n";
    this->CleanAdverts_();
    this->InitilizeAdverts_();
    current_advert_=adverts_.begin();
    return true;
  }


    std::time_t current_time=std::time(0);

     if(
      ((current_time-current_timestamp_)<current_advert_->get()->Get_advert_show_time())
          )
    {
     return false;
    }

  //if no adverts render all the time stub
  if(adverts_.size()-1==0){
    return false;
  }


  unsigned int i=1;//one because stub will always be
  while(i!=adverts_.size()){

    //be awere what .end() point on nullptr element in list/vector
    // need std::prev to decrement iterator
    //BE AWERE on standart C++17 better use std::Advance for incrementing and
    //decrement
    if(current_advert_==std::prev(adverts_.end(),1)){
      //point on second element in vector
      //because first is always stub
      current_advert_=std::next(adverts_.begin(),1);
    }
    else{ 
      //go to next element in colection
      current_advert_=std::next(current_advert_,1);
    }

    //check if advert ready to be shown
    if(current_advert_->get()->IsAdvertValid()&&
      current_advert_->get()->IsAdvertReady() && 
      current_advert_->get()->IsTimeReady(current_time)
      )
    {
     current_timestamp_=current_time;
     return true;
    }
  

    ++i;
  }
    current_timestamp_=current_time;
    //stub time to shown
    current_advert_=adverts_.begin();
  return true;
}

void WgHtmlAds::render() {
  WgBackground::render(); // if commented @ header and advert  block @ is
                          // tranperent

 //std::cout<<current_advert_->get()->Get_advert_title()<<std::endl;
  
  if(current_advert_->get()->RenderAdvert()!=true){
    RenderWidgetHeader(RENDERING_ERROR_HEADER_TEXT);
    std::cerr<<StrNow()
    <<"\tLibshape text must be here about adverts picture rendering error\n";
  }
  else{
      //~~~ render header
  RenderWidgetHeader((current_advert_->get()->Get_advert_title()).c_str());
  }

  //~~~~~ render shadows
  WgBackground::RenderOnlyShadows();
}
