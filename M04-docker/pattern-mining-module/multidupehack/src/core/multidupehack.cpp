// Copyright (C) 2013-2020 Loïc Cerf (lcerf@dcc.ufmg.br)

// This file is part of multidupehack.

// multidupehack is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

// multidupehack is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along with multidupehack.  If not, see <https://www.gnu.org/licenses/>.

#include <boost/program_options.hpp>
#include "sysexits.h"

#include "SkyPatternTree.h"

using namespace boost::program_options;

template<typename T> vector<T> getVectorFromString(const string& str)
{
  vector<T> tokens;
  T token;
  istringstream ss(str);
  while (ss >> token)
    {
      tokens.push_back(token);
    }
  return tokens;
}

template<typename T> vector<T> getSetFromString(const string& str)
{
  vector<T> tokens = getVectorFromString<T>(str);
  sort(tokens.begin(), tokens.end());
  unique(tokens.begin(), tokens.end());
  return tokens;
}

template<typename T> vector<vector<T>> getMatrixFromFile(const string& fileName)
{
  ifstream file;
  if (fileName == "-")
    {
      file.open("/dev/stdin");
    }
  else
    {
      file.open(fileName);
      if (!file)
	{
	  throw NoInputException(fileName.c_str());
	}
    }
  vector<vector<T>> matrix;
  while (!file.eof())
    {
      string rowString;
      getline(file, rowString);
      const vector<T> row = getVectorFromString<T>(rowString);
      if (!row.empty())
	{
	  matrix.push_back(row);
	}
    }
  file.close();
  return matrix;
}

int main(int argc, char* argv[])
{
  Tree* root;
  // Parsing the command line and the option file
  try
    {
      string dataFileName;
      string outputFileName;
      int maxArea = -1;
      string groupElementSeparator;
      string groupDimensionElementsSeparator;
      string pointElementSeparator;
      string pointDimensionSeparator;
      string valueElementSeparator;
      string valueDimensionSeparator;
      string sumValueFileName;
      string slopePointFileName;
      float minSum = 0;
      float minSlope = 0;
      options_description generic("Generic options");
      generic.add_options()
	("help,h", "produce help message")
	("hm", value<string>(), "display help module whose name (\"size-constraints\", \"group-constraints\", \"value-constraints\", \"point-constraints\" or \"io\") starts with the string in argument")
	("version,V", "display version information and exit")
	("opt", value<string>(), "set the option file name (by default [data-file].opt if present)");
      options_description basicConfig("Basic configuration (on the command line or in the option file)");
      basicConfig.add_options()
	("epsilon,e", value<string>(), "set noise tolerance bounds for elements in each attribute (by default 0 for every attribute)")
	("clique,c", value<string>(), "set attributes on which cliques are searched (0 being the first attribute)")
	("tau,t", value<string>(), "set maximal differences between two contiguous elements in numerical attributes or 0 for infinity/non-numerical attribute (by default 0 for every attribute)")
	("reduction,r", "do not compute ET-n-sets, only output the input data without the elements that cannot be in any ET-n-set given the size constraints")
	("unclosed,u", value<string>(), "set attributes in which the computed ET-n-sets need not be closed")
	("density,d", value<float>()->default_value(0), "set threshold between 0 (completely dense storage) and 1 (minimization of memory usage) to trigger a dense storage of the data")
	("out,o", value<string>(&outputFileName)->default_value("-"), "set output file name")
	("psky", "print pattern skyline whenever refined");
      options_description sizeConstraints("Size constraints (on the command line or in the option file)");
      sizeConstraints.add_options()
	("sizes,s", value<string>(), "set minimal sizes in each attribute of any computed ET-n-set (by default 0 for every attribute)")
	("Sizes,S", value<string>(), "set maximal sizes in each attribute of any computed ET-n-set (unconstrained by default)")
	("area,a", value<int>()->default_value(1), "set minimal area of any computed ET-n-set")
	("Area,A", value<int>(&maxArea), "set maximal area of any computed ET-n-set (unconstrained by default)")
	("sky-s", value<string>(), "set attributes whose sizes are to be maximized (0 being the first attribute)")
	("sky-S", value<string>(), "set attributes whose sizes are to be minimized (0 being the first attribute)")
	("sky-a", "maximize area")
	("sky-A", "minimize area");
      options_description groupConstraints("Group constraints (on the command line or in the option file)");
      groupConstraints.add_options()
	("groups,g", value<string>(), "set the names of the files describing every group (none by default)")
	("gs", value<string>(), "set minimal number of elements in each group (by default, all elements if no group measure is used, otherwise 0 for every group)")
	("gS", value<string>(), "set maximal number of elements in each group (unconstrained by default)")
	("gr", value<string>(), "set file name specifying minimal ratios between the number of elements in each group (by default 0 for every pair of groups)")
	("gps", value<string>(), "set file name specifying minimal Piatetsky-Shapiro's measures between the number of elements in each group (by default unconstrained for every pair of groups)")
	("gl", value<string>(), "set file name specifying minimal leverages between the number of elements in each group (by default unconstrained for every pair of groups)")
	("gf", value<string>(), "set file name specifying minimal forces between the number of elements in each group (by default 0 for every pair of groups)")
	("gyq", value<string>(), "set file name specifying minimal Yule's Q between the number of elements in each group (by default -1 for every pair of groups)")
	("gyy", value<string>(), "set file name specifying minimal Yule's Y between the number of elements in each group (by default -1 for every pair of groups)")
	("sky-gs", value<string>(), "set groups whose covers are to be maximized (0 being the first group)")
	("sky-gS", value<string>(), "set groups whose covers are to be minimized (0 being the first group)")
	("sky-gr", value<string>(), "set file name specifying the pairs of groups whose ratios of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gps", value<string>(), "set file name specifying the pairs of groups whose Piatetsky-Shapiro's measures of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gl", value<string>(), "set file name specifying the pairs of groups whose leverages of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gf", value<string>(), "set file name specifying the pairs of groups whose forces of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gyq", value<string>(), "set file name specifying the pairs of groups whose Yule's Q of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)")
	("sky-gyy", value<string>(), "set file name specifying the pairs of groups whose Yule's Y of their numbers of elements are to be maximized (0 indicates no maximization, another number indicates a maximization)");
      options_description valueConstraints("Value constraints (on the command line or in the option file)");
      valueConstraints.add_options()
	("sum-values,v", value<string>(&sumValueFileName), "set file name specifying, first, the ids of the k involved attributes (0 being the first attribute) and, then, k-tuples followed with values")
	("sum", value<float>(&minSum), "set minimal sum of the values specified with option --values and whose associated tuples can be made from an ET-n-set (unconstrained by default if --sky-sum in use, otherwise 0)")
	("sky-sum", "maximize sum");
      options_description pointConstraints("Point constraints (on the command line or in the option file)");
      pointConstraints.add_options()
	("slope-points", value<string>(&slopePointFileName), "set file name specifying, first, the ids of the k involved attributes (0 being the first attribute) and, then, k-tuples followed with 2D points")
	("slope", value<float>(&minSlope), "set minimal slope of the line fitting the points specified with option --slope-points and whose associated tuples can be made from an ET-n-set (unconstrained by default if --sky-slope in use, otherwise 0)")
	("sky-slope", "maximize slope");
      options_description io("Input/Output format (on the command line or in the option file)");
      io.add_options()
	("ids", value<string>()->default_value(" "), "set any character separating two attributes in input data")
	("ies", value<string>()->default_value(","), "set any character separating two elements in input data")
	("gds", value<string>(&groupDimensionElementsSeparator), "set any character separating the dimension from its elements in a group (by default same as --ids)")
	("ges", value<string>(&groupElementSeparator), "set any character separating two elements in a group (by default same as --ies)")
	("vds", value<string>(&valueDimensionSeparator), "set any character separating the dimensions in a value file (by default same as --ids)")
	("ves", value<string>(&valueElementSeparator), "set any character separating two elements in a value file (by default same as --ies)")
	("pds", value<string>(&pointDimensionSeparator), "set any character separating the dimensions in a point file (by default same as --ids)")
	("pes", value<string>(&pointElementSeparator), "set any character separating two elements in a point file (by default same as --ies)")
	("ods", value<string>()->default_value(" "), "set string separating two attributes in output data")
	("oes", value<string>()->default_value(","), "set string separating two elements in output data")
	("empty", value<string>()->default_value("ø"), " set string specifying an empty set in output data")
	("pn", "print absolute noise on every element in output data unless sky-patterns are searched (--pn has no effect)")
	("ens", value<string>()->default_value("#"), "set string separating every element from the noise on it")
	("ps", "print sizes in output data")
	("css", value<string>()->default_value(" : "), "set string separating ET-n-sets from sizes in output data")
	("ss", value<string>()->default_value(" "), "set string separating sizes of the different attributes in output data")
	("pa", "print areas in output data")
	("sas", value<string>()->default_value(" : "), "set string separating sizes from areas in output data");
      options_description hidden("Hidden options");
      hidden.add_options()
	("data-file", value<string>(&dataFileName)->default_value("/dev/stdin"), "set input data file");
      positional_options_description p;
      p.add("data-file", -1);
      options_description commandLineOptions;
      commandLineOptions.add(generic).add(basicConfig).add(sizeConstraints).add(groupConstraints).add(valueConstraints).add(pointConstraints).add(io).add(hidden);
      variables_map vm;
      store(command_line_parser(argc, argv).options(commandLineOptions).positional(p).run(), vm);
      notify(vm);
      if (vm.count("help"))
	{
	  cout << "Usage: multidupehack [options] [data-file]\n" << generic << basicConfig << "\nReport bugs to: lcerf@dcc.ufmg.br\n";
	  return EX_OK;
	}
      if (vm.count("hm"))
	{
	  if (vm["hm"].as<string>() == string("size-constraints").substr(0, vm["hm"].as<string>().size()))
	    {
	      cout << sizeConstraints;
	      return EX_OK;
	    }
	  if (vm["hm"].as<string>() == string("group-constraints").substr(0, vm["hm"].as<string>().size()))
	    {
	      cout << groupConstraints;
	      return EX_OK;
	    }
	  if (vm["hm"].as<string>() == string("value-constraints").substr(0, vm["hm"].as<string>().size()))
	    {
	      cout << valueConstraints;
	      return EX_OK;
	    }
	  if (vm["hm"].as<string>() == string("point-constraints").substr(0, vm["hm"].as<string>().size()))
	    {
	      cout << pointConstraints;
	      return EX_OK;
	    }
	  if (vm["hm"].as<string>() == string("io").substr(0, vm["hm"].as<string>().size()))
	    {
	      cout << io;
	      return EX_OK;
	    }
	  cerr << "Unknown help module " << vm["hm"].as<string>() << ": valid modules are \"size-constraints\", \"group-constraints\", \"value-constraints\", \"point-constraints\" and \"io\"\n";
	  return EX_USAGE;
	}
      if (vm.count("version"))
	{
	  cout << "multidupehack version 0.26.7.4\nCopyright (C) 2013-2021 Loïc Cerf.\nLicense GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n";
	  return EX_OK;
	}
      ifstream optionFile;
      if (vm.count("opt"))
	{
	  if (vm["opt"].as<string>() == "-")
	    {
	      optionFile.open("/dev/stdin");
	    }
	  else
	    {
	      optionFile.open(vm["opt"].as<string>());
	      if (!optionFile)
		{
		  throw NoInputException(vm["opt"].as<string>().c_str());
		}
	    }
	}
      else
	{
	  if (vm.count("data-file"))
	    {
	      optionFile.open((dataFileName + ".opt").c_str());
	    }
	}
      options_description config;
      config.add(basicConfig).add(sizeConstraints).add(groupConstraints).add(valueConstraints).add(pointConstraints).add(io).add(hidden);
      store(parse_config_file(optionFile, config), vm);
      optionFile.close();
      notify(vm);
      vector<unsigned int> cliqueDimensions;
      if (vm.count("clique"))
	{
	  cliqueDimensions = getSetFromString<unsigned int>(vm["clique"].as<string>());
	  if (cliqueDimensions.size() != 2)
	    {
	      throw UsageException("clique option should provide two different attributes ids!");
	    }
	}
      if (vm["density"].as<float>() < 0 || vm["density"].as<float>() > 1)
	{
	  throw UsageException("density option should provide a float between 0 and 1!");
	}
      if (vm["area"].as<int>() < 0)
	{
	  throw UsageException("area option should provide a positive integer!");
	}
      vector<string> groupFileNames;
      if (vm.count("groups"))
	{
	  for (const string& groupFileName : tokenizer<char_separator<char>>(vm["groups"].as<string>(), char_separator<char>(" ")))
	    {
	      if (groupFileName == "-")
		{
		  groupFileNames.push_back("/dev/stdin");
		}
	      else
		{
		  groupFileNames.push_back(groupFileName);
		}
	    }
	}
      vector<unsigned int> groupMinSizes;
      if (vm.count("gs"))
	{
	  groupMinSizes = getVectorFromString<unsigned int>(vm["gs"].as<string>());
	  if (groupMinSizes.size() > groupFileNames.size())
	    {
	      throw UsageException(("gs option provides " + lexical_cast<string>(groupMinSizes.size()) + " sizes but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<unsigned int> groupMaxSizes;
      if (vm.count("gS"))
	{
	  groupMaxSizes = getVectorFromString<unsigned int>(vm["gS"].as<string>());
	  if (groupMaxSizes.size() > groupFileNames.size())
	    {
	      throw UsageException(("gS option provides " + lexical_cast<string>(groupMaxSizes.size()) + " sizes but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMinRatios;
      if (vm.count("gr"))
	{
	  groupMinRatios = getMatrixFromFile<float>(vm["gr"].as<string>());
	  if (groupMinRatios.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gr option has " + lexical_cast<string>(groupMinRatios.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMinPiatetskyShapiros;
      if (vm.count("gps"))
	{
	  groupMinPiatetskyShapiros = getMatrixFromFile<float>(vm["gps"].as<string>());
	  if (groupMinPiatetskyShapiros.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gps option has " + lexical_cast<string>(groupMinPiatetskyShapiros.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMinLeverages;
      if (vm.count("gl"))
	{
	  groupMinLeverages = getMatrixFromFile<float>(vm["gl"].as<string>());
	  if (groupMinLeverages.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gl option has " + lexical_cast<string>(groupMinLeverages.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMinForces;
      if (vm.count("gf"))
	{
	  groupMinForces = getMatrixFromFile<float>(vm["gf"].as<string>());
	  if (groupMinForces.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gf option has " + lexical_cast<string>(groupMinForces.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMinYulesQs;
      if (vm.count("gyq"))
	{
	  groupMinYulesQs = getMatrixFromFile<float>(vm["gyq"].as<string>());
	  if (groupMinYulesQs.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gyq option has " + lexical_cast<string>(groupMinYulesQs.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMinYulesYs;
      if (vm.count("gyy"))
	{
	  groupMinYulesYs = getMatrixFromFile<float>(vm["gyy"].as<string>());
	  if (groupMinYulesYs.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with gyy option has " + lexical_cast<string>(groupMinYulesYs.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<unsigned int> groupMaximizedSizes;
      if (vm.count("sky-gs"))
	{
	  groupMaximizedSizes = getSetFromString<unsigned int>(vm["sky-gs"].as<string>());
	  sort(groupMaximizedSizes.begin(), groupMaximizedSizes.end());
	  if (groupMaximizedSizes.back() >= groupFileNames.size())
	    {
	      throw UsageException(("sky-gs option should provide group ids between 0 and " + lexical_cast<string>(groupFileNames.size() - 1)).c_str());
	    }
	}
      vector<unsigned int> groupMinimizedSizes;
      if (vm.count("sky-gS"))
	{
	  groupMinimizedSizes = getSetFromString<unsigned int>(vm["sky-gS"].as<string>());
	  sort(groupMinimizedSizes.begin(), groupMinimizedSizes.end());
	  if (groupMinimizedSizes.back() >= groupFileNames.size())
	    {
	      throw UsageException(("sky-gS option should provide group ids between 0 and " + lexical_cast<string>(groupFileNames.size() - 1)).c_str());
	    }
	}
      vector<vector<float>> groupMaximizedRatios;
      if (vm.count("sky-gr"))
	{
	  groupMaximizedRatios = getMatrixFromFile<float>(vm["sky-gr"].as<string>());
	  if (groupMaximizedRatios.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gr option has " + lexical_cast<string>(groupMaximizedRatios.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMaximizedPiatetskyShapiros;
      if (vm.count("sky-gps"))
	{
	  groupMaximizedPiatetskyShapiros = getMatrixFromFile<float>(vm["sky-gps"].as<string>());
	  if (groupMaximizedPiatetskyShapiros.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gps option has " + lexical_cast<string>(groupMaximizedPiatetskyShapiros.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMaximizedLeverages;
      if (vm.count("sky-gl"))
	{
	  groupMaximizedLeverages = getMatrixFromFile<float>(vm["sky-gl"].as<string>());
	  if (groupMaximizedLeverages.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gl option has " + lexical_cast<string>(groupMaximizedLeverages.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMaximizedForces;
      if (vm.count("sky-gf"))
	{
	  groupMaximizedForces = getMatrixFromFile<float>(vm["sky-gf"].as<string>());
	  if (groupMaximizedForces.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gf option has " + lexical_cast<string>(groupMaximizedForces.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMaximizedYulesQs;
      if (vm.count("sky-gyq"))
	{
	  groupMaximizedYulesQs = getMatrixFromFile<float>(vm["sky-gyq"].as<string>());
	  if (groupMaximizedYulesQs.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gyq option has " + lexical_cast<string>(groupMaximizedYulesQs.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      vector<vector<float>> groupMaximizedYulesYs;
      if (vm.count("sky-gyy"))
	{
	  groupMaximizedYulesYs = getMatrixFromFile<float>(vm["sky-gyy"].as<string>());
	  if (groupMaximizedYulesYs.size() > groupFileNames.size())
	    {
	      throw UsageException(("file set with sky-gyy option has " + lexical_cast<string>(groupMaximizedYulesYs.size()) + " non-empty rows but groups option only defines " + lexical_cast<string>(groupFileNames.size()) + " groups!").c_str());
	    }
	}
      if (sumValueFileName.empty())
	{
	  if (vm.count("sum"))
	    {
	      throw UsageException("sum option without sum-values option!");
	    }
	  if (vm.count("sky-sum"))
	    {
	      throw UsageException("sky-sum option without sum-values option!");
	    }
	}
      else
	{
	  if (vm["sum-values"].as<string>() == "-")
	    {
	      sumValueFileName = "/dev/stdin";
	    }
	}
      if (slopePointFileName.empty())
	{
	  if (vm.count("slope"))
	    {
	      throw UsageException("slope option without slope-points option!");
	    }
	  if (vm.count("sky-slope"))
	    {
	      throw UsageException("sky-slope option without slope-points option!");
	    }
	}
      else
	{
	  if (vm["slope-points"].as<string>() == "-")
	    {
	      slopePointFileName = "/dev/stdin";
	    }
	}
      if (vm["data-file"].as<string>() == "-")
	{
	  dataFileName = "/dev/stdin";
	}
      if (vm["out"].as<string>() == "-")
	{
	  outputFileName = "/dev/stdout";
	}
      vector<unsigned int> minSizes;
      if (vm.count("sizes"))
	{
	  minSizes = getVectorFromString<unsigned int>(vm["sizes"].as<string>());
	}
      else
	{
	  if (!vm.count("area"))
	    {
	      cerr << "Warning: Either you really know what you are doing or you forgot the sizes\noption or the area option (or both)\n";
	    }
	}
      vector<double> epsilonVector;
      if (vm.count("epsilon"))
	{
	  epsilonVector = getVectorFromString<double>(vm["epsilon"].as<string>());
	}
      vector<double> tauVector;
      if (vm.count("tau"))
	{
	  tauVector = getVectorFromString<double>(vm["tau"].as<string>());
	}
      vector<unsigned int> unclosedDimensions;
      if (vm.count("unclosed"))
	{
	  unclosedDimensions = getVectorFromString<unsigned int>(vm["unclosed"].as<string>());
	}
      vector<unsigned int> maxSizes;
      if (vm.count("Sizes"))
	{
	  maxSizes = getVectorFromString<unsigned int>(vm["Sizes"].as<string>());
	}
      vector<unsigned int> maximizedSizeDimensions;
      if (vm.count("sky-s"))
	{
	  maximizedSizeDimensions = getSetFromString<unsigned int>(vm["sky-s"].as<string>());
	}
      vector<unsigned int> minimizedSizeDimensions;
      if (vm.count("sky-S"))
	{
	  minimizedSizeDimensions = getSetFromString<unsigned int>(vm["sky-S"].as<string>());
	}
      if (vm.count("sky-sum") && !vm.count("sum"))
	{
	  minSum = -numeric_limits<float>::infinity();
	}
      if (vm.count("sky-slope") && !vm.count("slope"))
	{
	  minSlope = -numeric_limits<float>::infinity();
	}
      if (!vm.count("gds"))
	{
	  groupDimensionElementsSeparator = vm["ids"].as<string>();
	}
      if (!vm.count("ges"))
	{
	  groupElementSeparator = vm["ies"].as<string>();
	}
      if (!vm.count("vds"))
	{
	  valueDimensionSeparator = vm["ids"].as<string>();
	}
      if (!vm.count("ves"))
	{
	  valueElementSeparator = vm["ies"].as<string>();
	}
      if (!vm.count("pds"))
	{
	  pointDimensionSeparator = vm["ids"].as<string>();
	}
      if (!vm.count("pes"))
	{
	  pointElementSeparator = vm["ies"].as<string>();
	}
      if (vm.count("sky-s") || vm.count("sky-S") || vm.count("sky-a") || vm.count("sky-A") || vm.count("sky-gs") || vm.count("sky-gS") || vm.count("sky-gr") || vm.count("sky-gps") || vm.count("sky-gl") || vm.count("sky-gf") || vm.count("sky-gyq") || vm.count("sky-gyy") || vm.count("sky-sum") || vm.count("sky-slope"))
	{
	  root = new SkyPatternTree(dataFileName.c_str(), vm["density"].as<float>(), epsilonVector, cliqueDimensions, tauVector, minSizes, vm["area"].as<int>(), vm.count("reduction"), unclosedDimensions, vm["ies"].as<string>().c_str(), vm["ids"].as<string>().c_str(), outputFileName.c_str(), vm["ods"].as<string>().c_str(), vm["css"].as<string>().c_str(), vm["ss"].as<string>().c_str(), vm["sas"].as<string>().c_str(), vm.count("ps"), vm.count("pa"), vm.count("psky"));
	  try
	    {
	      static_cast<SkyPatternTree*>(root)->initMeasures(maxSizes, maxArea, maximizedSizeDimensions, minimizedSizeDimensions, vm.count("sky-a"), vm.count("sky-A"), groupFileNames, groupMinSizes, groupMaxSizes, groupMinRatios, groupMinPiatetskyShapiros, groupMinLeverages, groupMinForces, groupMinYulesQs, groupMinYulesYs, groupElementSeparator.c_str(), groupDimensionElementsSeparator.c_str(), groupMaximizedSizes, groupMinimizedSizes, groupMaximizedRatios, groupMaximizedPiatetskyShapiros, groupMaximizedLeverages, groupMaximizedForces, groupMaximizedYulesQs, groupMaximizedYulesYs, sumValueFileName.c_str(), minSum, valueElementSeparator.c_str(), valueDimensionSeparator.c_str(), vm.count("sky-sum"), slopePointFileName.c_str(), minSlope, pointElementSeparator.c_str(), pointDimensionSeparator.c_str(), vm.count("sky-slope"), vm["density"].as<float>());
	    }
	  catch (std::exception& e)
	    {
	      delete root;
	      rethrow_exception(current_exception());
	    }
	}
      else
	{
	  root = new Tree(dataFileName.c_str(), vm["density"].as<float>(), epsilonVector, cliqueDimensions, tauVector, minSizes, vm["area"].as<int>(), vm.count("reduction"), unclosedDimensions, vm["ies"].as<string>().c_str(), vm["ids"].as<string>().c_str(), outputFileName.c_str(), vm["ods"].as<string>().c_str(), vm["css"].as<string>().c_str(), vm["ss"].as<string>().c_str(), vm["sas"].as<string>().c_str(), vm.count("ps"), vm.count("pa"));
	  try
	    {
	      root->initMeasures(maxSizes, maxArea, groupFileNames, groupMinSizes, groupMaxSizes, groupMinRatios, groupMinPiatetskyShapiros, groupMinLeverages, groupMinForces, groupMinYulesQs, groupMinYulesYs, groupElementSeparator.c_str(), groupDimensionElementsSeparator.c_str(), sumValueFileName.c_str(), minSum, valueElementSeparator.c_str(), valueDimensionSeparator.c_str(), slopePointFileName.c_str(), minSlope, pointElementSeparator.c_str(), pointDimensionSeparator.c_str(), vm["density"].as<float>());
	    }
	  catch (std::exception& e)
	    {
	      delete root;
	      rethrow_exception(current_exception());
	    }
	}
      Attribute::setOutputFormat(vm["oes"].as<string>().c_str(), vm["empty"].as<string>().c_str(), vm["ens"].as<string>().c_str(), vm.count("pn"));
    }
  catch (unknown_option& e)
    {
      cerr << "Unknown option!\n";
      return EX_USAGE;
    }
  catch (NoInputException& e)
    {
      cerr << e.what() << '\n';
      return EX_NOINPUT;
    }
  catch (UsageException& e)
    {
      cerr << e.what() << '\n';
      return EX_USAGE;
    }
  catch (DataFormatException& e)
    {
      cerr << e.what() << '\n';
      return EX_DATAERR;
    }
  catch (NoOutputException& e)
    {
      cerr << e.what() << '\n';
      return EX_CANTCREAT;
    }
  root->mine();
  root->terminate();
  delete root;
  return EX_OK;
}
