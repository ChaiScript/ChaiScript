// This file is distributed under the BSD License.
// See "license.txt" for details.
// Copyright 2009, Jonathan Turner (jturner@minnow-lang.org)
// and Jason Turner (lefticus@gmail.com)
// http://www.chaiscript.com

#include <iostream>

#include "chaiscript.hpp"
#include "function_call.hpp"
#include <boost/function.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>


std::string load_text_file(const std::string &filename)
{
  std::ifstream infile(filename.c_str());

  std::string str;

  std::string result;

  while (std::getline(infile, str))
  {
    result += str + "\n";
  }

  return result;
}

std::vector<dispatchkit::Boxed_Value> regex_search(const std::string &str, const std::string &regex)
{
  boost::smatch matches;
  boost::regex_search(str, matches, boost::regex(regex));

  std::vector<dispatchkit::Boxed_Value> results;

  for (unsigned int i = 0; i < matches.size(); ++i)
  {
    results.push_back(dispatchkit::Boxed_Value(std::string(matches.str(i))));
  }

  return results;
}

struct Sensor_Manager
{
  struct Sensor
  {
    int milliseconds;
    dispatchkit::Boxed_Value state_object;
    boost::function<double (dispatchkit::Boxed_Value)> sensor;
    boost::posix_time::ptime next_run;

    Sensor(int t_milliseconds, dispatchkit::Boxed_Value t_state_object, 
        boost::function<double (dispatchkit::Boxed_Value)> t_sensor)
      : milliseconds(t_milliseconds), state_object(t_state_object), sensor(t_sensor),
        next_run(boost::posix_time::microsec_clock::universal_time() 
            + boost::posix_time::milliseconds(milliseconds))
    {
    }

    std::pair<boost::posix_time::ptime, double> get_value()
    {
      next_run = boost::posix_time::microsec_clock::universal_time() 
            + boost::posix_time::milliseconds(milliseconds);

      return std::make_pair(boost::posix_time::microsec_clock::universal_time(),
          sensor(state_object));
    }
  };

  std::map<std::string, Sensor> m_sensors;

  //sensor_manager.add_sensor("CPU", 1000, global_state, function(state) { update_state(state); state["CPU"]; } )
  void add_sensor(const std::string &t_name, int t_milliseconds, dispatchkit::Boxed_Value t_state_object,
      boost::shared_ptr<dispatchkit::Proxy_Function> t_func)
  {
    m_sensors.insert(
        std::make_pair(t_name, 
          Sensor(t_milliseconds, t_state_object, 
            dispatchkit::build_function_caller<double (dispatchkit::Boxed_Value)>(t_func)
            )
          )
        );
  }


  std::vector<std::pair<std::string, double> > run_sensors()
  {
    std::vector<std::pair<std::string, double> > results;

    boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());

    for (std::map<std::string, Sensor>::iterator itr = m_sensors.begin();
         itr != m_sensors.end();
         ++itr)
    {
      if (itr->second.next_run <= t)
      {
        results.push_back(std::make_pair(itr->first, itr->second.get_value().second));
      }
    }

    return results;
  }

};


int main(int argc, char *argv[]) {

  chaiscript::ChaiScript_Engine chai;

  Sensor_Manager sensor_manager;
  chai.get_eval_engine().add_object("sensor_manager", boost::ref(sensor_manager));

  dispatchkit::register_function(chai.get_eval_engine(), &Sensor_Manager::add_sensor, "add_sensor");
  dispatchkit::register_function(chai.get_eval_engine(), &regex_search, "regex_search");
  dispatchkit::register_function(chai.get_eval_engine(), &load_text_file, "load_text_file");



  for (int i = 1; i < argc; ++i) {
    try {
      chai.evaluate_file(argv[i]);
    }
    catch (std::exception &e) {
      std::cerr << "Could not open: " << argv[i] << std::endl;
      exit(1);
    }
  }

  while (true)
  {
    usleep(1000);
    std::vector<std::pair<std::string, double> > sensor_data = sensor_manager.run_sensors();

    for (std::vector<std::pair<std::string, double> >::iterator itr = sensor_data.begin();
         itr != sensor_data.end();
         ++itr)
    {
      std::cout << "Sensor: " << itr->first << " value: " << itr->second << std::endl;
    }
  }
}

