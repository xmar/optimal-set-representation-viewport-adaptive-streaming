#include "Optimal.hpp"

#include "common/ConfigArgs.hpp"
#include "common/Common.hpp"
#include "AreaSet.hpp"
#include "PrecomputeSegmentsIntersections.hpp"
#include "PrecomputedAllowedVersion.hpp"

#include <ilcplex/ilocplex.h>

#include <fstream>
#include <random>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

#include <iostream>

using namespace IMT;

typedef IloArray<IloNumArray> FloatMatrix;
typedef IloArray<IloNumVarArray> VarMatrix;
typedef IloArray<IloIntVarArray> VarIntMatrix;

#define TARGET_BITRATE 4*PI

std::tuple<Float,Float> GetSurfaceBitrateQerOut(double Sqer, std::shared_ptr<ConfigArgs const> configArgs)
{
  const auto& minBitrate = configArgs->minSurfaceBitrate;
  const auto& maxBitrate = configArgs->maxSurfaceBitrate;
  const auto& r = configArgs->bitrateRatio;
  constexpr Float B = 4*PI;
  Float midLimite = (4*PI - minBitrate*4*PI)/(maxBitrate-minBitrate);
  Float y = (4*PI-Sqer)/(4*PI-Sqer+Sqer*r);
  Float firstL = (4*PI*maxBitrate-B*r)/((1-r)*maxBitrate);
  Float lastL = (4*PI*minBitrate-B)/((1-r)*minBitrate);

  if (Sqer <= midLimite)
  {
    Float Bqer(-1);
    Float Bout(-1);
    if (Sqer > firstL)
    {
      Float bint = (B - (maxBitrate * Sqer)) / (4*PI - Sqer);
      Float extraPre = (maxBitrate - r * bint)* Sqer;
      Bqer = r*bint + extraPre*(1-y)/Sqer;
      Bout = bint + extraPre*y/(4*PI-Sqer);
    }
    else
    {
      Bqer = maxBitrate;
      Bout = (4*PI - maxBitrate * Sqer)/(4*PI - Sqer);
    }

    if (Bout + 0.001 < minBitrate)
    {
      std::cout.precision(8);
      std::cout << "Error: Bout smaller than Bmin: " << (Sqer > firstL ? ". " : "* ") << Bout << " < " << minBitrate << std::endl;
    }
    if (Bqer - 0.001 > maxBitrate)
    {
      std::cout.precision(8);
      std::cout << "Error: Bqer greater than Bmax: " << (Sqer > firstL ? ". " : "* ") << Bqer << " > " << maxBitrate << std::endl;
    }
    if (Bqer - 0.001 > r*Bout)
    {
      std::cout.precision(8);
      std::cout << "Error: Bqer greater than Rb*Bout: " << (Sqer > firstL ? ". " : "* ") << Bqer << " > " << r*Bout << std::endl;
    }
    if (Bqer < 1 || Bout > 1)
    {
      std::cout.precision(8);
      std::cout << "Error: Bqer or Bout too big/small: " << (Sqer > firstL ? ". " : "* ") << Bqer << " | " << Bout << std::endl;
    }
    if (std::isnan(Bqer))
    {
      Bqer = 0;
    }
    if (std::isnan(Bout))
    {
      Bout = 0;
    }
    return std::forward_as_tuple(Bqer, Bout);
  }
  else
  {
    Float Bqer(-1);
    Float Bout(-1);
    if (Sqer < lastL)
    {
      Float extra = B - (minBitrate * (4*PI - Sqer)) - (minBitrate * r * Sqer);
      Bqer = (r * minBitrate) + extra*(1-y)/Sqer;
      Bout = minBitrate + extra*y/(4*PI-Sqer);
    }
    else
    {
      Bqer = (4*PI - minBitrate * (4*PI-Sqer))/Sqer;
      Bout = minBitrate;
    }

    if (Bout + 0.001 < minBitrate)
    {
      std::cout.precision(8);
      std::cout << "Error: Bout smaller than Bmin: " << (Sqer < lastL ? ", " : "# ") << Bout << " < " << minBitrate << std::endl;
    }
    if (Bqer - 0.001 > maxBitrate)
    {
      std::cout.precision(8);
      std::cout << "Error: Bqer greater than Bmax: " << (Sqer < lastL ? ", " : "# ") << Bqer << " > " << maxBitrate << std::endl;
    }
    if (Bqer - 0.001 > r*Bout)
    {
      std::cout.precision(8);
      std::cout << "Error: Bqer greater than Rb*Bout: " << (Sqer < lastL ? ", " : "# ") << Bqer << " > " << r*Bout << std::endl;
    }
    if (Bqer < 1 || Bout > 1)
    {
      std::cout.precision(8);
      std::cout << "Error: Bqer or Bout too big/small: " << (Sqer < lastL ? ", " : "# ") << Bqer << " | " << Bout << std::endl;
    }
    if (std::isnan(Bqer))
    {
      Bqer = 0;
    }
    if (std::isnan(Bout))
    {
      Bout = 0;
    }
    return std::forward_as_tuple(Bqer, Bout);
  }
}

void Optimal::Run(void)
{

  std::string outputDir = m_conf->pathToOutputDir + "/" + std::to_string(m_conf->nbQer)+"_"+ std::to_string(m_conf->nbHPixels)+"x"+std::to_string(m_conf->nbVPixels)+"_"+std::to_string(m_conf->minSurfaceBitrate)+"_"+std::to_string(m_conf->maxSurfaceBitrate)+"_"+std::to_string(m_conf->segmentDuration)+"s";
  if (m_conf->useTile)
  {
    outputDir += "Tiled"+ std::to_string(m_conf->nbHTiles)+"x"+std::to_string(m_conf->nbVTiles);
  }

  if (!fs::is_directory(m_conf->pathToOutputDir))
  {
    fs::create_directory(m_conf->pathToOutputDir);
  }
  if (!fs::is_directory(outputDir))
  {
    fs::create_directory(outputDir);
  }

  std::vector<double> sqer_vec;
  std::vector<double> n_vec;
  unsigned int testCount = 0;
  unsigned int totalTest = 0;
  for(auto videoId: m_psi->GetVideoIdVect())
  {
    for (auto segmentId: m_psi->GetSegIdVect(videoId))
    {
      ++totalTest;
    }
  }
  auto Percentile = [](std::vector<double>& vect, double percentile)
  {
    if (percentile == 0)
    {
      return vect[0];
    }
    else if(percentile == 100)
    {
      return vect[vect.size()-1];
    }
    else
    {
      Float dist = percentile*vect.size()/100.f;
      unsigned int down = std::floor(dist);
      unsigned int up = down+1;
      return vect[down] * (1-(dist-down)) + vect[up] * (1-(up-dist));
    }
  };

  //the random device that will seed the generator
  std::random_device seeder;
  //then make a mersenne twister engine
  std::mt19937 engine(seeder());


  //Vector that store the solutions
  std::vector<double> optiVisibleB;
  std::vector<double> heuristicVisibleB;
  std::vector<double> randomVisibleB;
  std::vector<double> versionSize;

  //Start processing loop
  for(auto videoId: m_psi->GetVideoIdVect())
  {
    for (auto segmentId: m_psi->GetSegIdVect(videoId))
    {
      std::cout << "Start optimization for " << videoId << " segment " << segmentId << std::endl;

      IloEnv env;
      IloModel model(env);

      unsigned nbArea = m_areaSet->GetAreas().size();
      auto psi_vid = m_psi->FilterVidSegId(videoId, segmentId);
      unsigned nbUser = psi_vid->GetSegments().size();
      unsigned nbVersion = m_pav->GetAllowedVersionVector().size();
      unsigned nbMaxVersion = m_conf->nbQer;

      try{
        std::cout << "Init constants" << std::endl;
        //const values:
        // FloatMatrix b(env, nbVersion);
        // for (unsigned r = 0; r < nbVersion; ++r)
        // {
        //   b[r] = IloNumArray(env, nbArea);
        //   for (unsigned a = 0; a < nbArea; ++a)
        //   {
        //     b[r][a] = 0;
        //   }
        // }
        std::vector<std::tuple<double, double>> b;

        FloatMatrix v(env, nbUser);
        for (unsigned u = 0; u < nbUser; ++u)
        {
          v[u] = IloNumArray(env, nbArea);
          for (unsigned a = 0; a < nbArea; ++a)
          {
            v[u][a] = 0;
          }
        }

        FloatMatrix v_b(env, nbUser);
        for (unsigned u = 0; u < nbUser; ++u)
        {
          v_b[u] = IloNumArray(env, nbVersion);
          for (unsigned r = 0; r < nbVersion; ++r)
          {
            v_b[u][r] = 0;
          }
        }

        std::cout << "Nb user = " << nbUser << std::endl;

        unsigned u = 0;
        for (auto const& s: psi_vid->GetSegments())
        {
          for (auto const& visibility: s->GetVisibilityVect())
          {
            for (unsigned a = 0; a < nbArea; ++a)
            {
              v[u][a] += visibility[a] ? 1 : 0;
            }
          }
          ++u;
          if (u >= nbUser) {break;}
        }

        double max_b_qer(0.0);
        double max_b_out(0.0);
        double min_b_qer(4*PI);
        double min_b_out(4*PI);
        for (unsigned r = 0; r < nbVersion; ++r)
        {
          double Squer(0.0);
          for (auto a = 0; a < nbArea; ++a)
          {
            if (m_pav->GetAllowedVersionVector()[r][a] == 1)
            {
              Squer += 4*PI/nbArea;
            }
          }
          double b_qer(0); double b_out(0);
          std::tie(b_qer,b_out) = GetSurfaceBitrateQerOut(Squer, m_conf);

          for (unsigned u = 0; u < nbUser; ++u)
          {
            auto sum = IloSum(v[u]);
            for (auto a = 0; a < nbArea; ++a)
            {
              v_b[u][r] += v[u][a] * (m_pav->GetAllowedVersionVector()[r][a] == 1 ? b_qer : b_out) / sum;
            }
          }

          max_b_qer = std::max(max_b_qer, b_qer);
          max_b_out = std::max(max_b_out, b_out);
          min_b_qer = std::min(min_b_qer, b_qer);
          min_b_out = std::min(min_b_out, b_out);
        }
        v.end();

        std::cout << "Nb views = " << psi_vid->NbView() << std::endl;


        //Create variables
        IloIntVarArray c(env, nbVersion, 0, 1);
        VarIntMatrix s(env, nbVersion);
        for (unsigned r = 0; r < nbVersion; ++r)
        {
          c[r].setName((std::string("c[r=") +std::to_string(r)+ std::string("]")).c_str());
          s[r] = IloIntVarArray(env, nbUser, 0, 1);
          for (unsigned u = 0; u  < nbUser; ++u)
          {
            s[r][u].setName((std::string("c[r=") +std::to_string(r)+ std::string("][u=") +std::to_string(u)+ std::string("]")).c_str());
          }
        }
        // VarMatrix b_selected(env, nbUser);
        // for (unsigned u = 0; u  < nbUser; ++u)
        // {
        //   b_selected[u] = IloNumVarArray(env, nbArea, min_b_out, max_b_qer, ILOFLOAT);
        //   for (auto a = 0; a < nbArea; ++a)
        //   {
        //     b_selected[u][a].setName((std::string("b_selected[u=") +std::to_string(u)+ std::string("][a=") +std::to_string(a)+ std::string("]")).c_str());
        //   }
        // }

        std::cout << "Generate constraints" << std::endl;

        // //Constraints 1
        // for (unsigned u = 0; u  < nbUser; ++u)
        // {
        //   for (auto a = 0; a < nbArea; ++a)
        //   {
        //       IloExpr selected_bitrate(env);
        //       for (unsigned r = 0; r < nbVersion; ++r)
        //       {
        //         selected_bitrate += s[r][u] * b[r][a];
        //       }
        //       IloConstraint ic = b_selected[u][a] == selected_bitrate;
        //       ic.setName((std::string("Const1_[u=") +std::to_string(u)+ std::string("][a=") +std::to_string(a)+ std::string("]")).c_str());
        //       model.add(ic);
        //   }
        // }

        //Constraints 2
        for (unsigned u = 0; u  < nbUser; ++u)
        {
          IloExpr sum(env);
          for (unsigned r = 0; r < nbVersion; ++r)
          {
            sum += s[r][u];
          }
          IloConstraint ic = sum == IloInt(1);
          ic.setName((std::string("Const2_[u=") +std::to_string(u)+ std::string("]")).c_str());
          model.add(ic);
        }

        //Constraints 3
        for (unsigned u = 0; u  < nbUser; ++u)
        {
          for (unsigned r = 0; r < nbVersion; ++r)
          {
            IloConstraint ic = s[r][u] <= c[r];
            ic.setName((std::string("Const3_[u=") +std::to_string(u)+ std::string("][r=") +std::to_string(r)+ std::string("]")).c_str());
            model.add(ic);
          }
        }

        //Constraints 5
        for (unsigned r = 0; r < nbVersion; ++r)
        {
          IloConstraint ic = IloSum(s[r]) >= c[r];
          ic.setName((std::string("Const5_[r=") +std::to_string(r)+ std::string("]")).c_str());
          model.add(ic);
        }

        //Constraint 4
        {
          IloConstraint ic = IloSum(c) <= IloInt(nbMaxVersion);
          ic.setName((std::string("Const4")).c_str());
          model.add(ic);
        }

      std::cout << "Generate objective" << std::endl;

      //Objective
      IloExpr sc(env);
      for (unsigned u = 0; u < nbUser; ++u)
      {
        // for (unsigned a = 0; a < nbArea; ++a)
        // {
        //   sc += (v[u][a] * b_selected[u][a] / IloSum(v[u]))/IloInt(nbUser);
        // }
        for (unsigned r = 0; r < nbVersion; ++r)
        {
          sc += s[r][u] * v_b[u][r]/IloInt(nbUser);
        }
      }
      model.add(IloMaximize(env, sc));

      std::cout << std::endl << "Ready to run" << std::endl;

      IloCplex cplex(model);
      cplex.setParam(IloCplex::EpGap, m_conf->epGap);
      cplex.setParam(IloCplex::Threads, m_conf->nbThread);
      // cplex.setParam(IloCplex::NodeFileInd,3);
      // cplex.setParam(IloCplex::WorkDir,".");
      // cplex.setParam(IloCplex::MemoryEmphasis,true);
      cplex.setParam(IloCplex::DataCheck, true);

      std::string outputSolPath = outputDir+"/"+videoId+"_"+segmentId+"_solution.sol";

      if (fs::exists(outputSolPath))
      {
        std::cout << "Load solution" << std::endl;

        cplex.readSolution(outputSolPath.c_str());

        std::cout << "Done" << std::endl;
      }

      // cplex.exportModel((outputDir+"/"+videoId+"_"+segmentId+"_model.lp").c_str());
      // cplex.exportModel((outputDir+"/"+videoId+"_"+segmentId+"_model.mps").c_str());

      std::cout << "Start solving" << std::endl;

      cplex.solve();

      std::cout << "Done" << std::endl;

      cplex.writeSolution(outputSolPath.c_str());




      env.out() << cplex.getStatus() << std::endl;
      env.out() << cplex.getObjValue() << std::endl;


      std::cout << "====== Solution ======" << std::endl;
      IloNumArray c_sol(env);
      cplex.getValues(c_sol, c);
      std::cout << "Nb max version: " << nbMaxVersion << std::endl;
      std::vector<unsigned> offeredVersion;
      for (unsigned r = 0; r < nbVersion; ++r)
      {
        if (c_sol[r] > 0.5)
        {
          m_pav->HeatVersion(r);
          offeredVersion.push_back(r);
          auto const& version = m_pav->GetAllowedVersionVector()[r];
          versionSize.push_back(version.GetSize());
          std::cout << "Version " << r << ": (" << version.GetTheta() << ", " << version.GetPhi() << ", "  << version.GetHDim() << ", "  << version.GetVDim() << ")" << std::endl;
          IloNumArray s_sol(env);
          cplex.getValues(s_sol, s[r]);
          std::cout << "Selected by: " << std::endl;
          for (unsigned u = 0; u < nbUser; ++u)
          {
            if (s_sol[u] > 0.5)
            {
              std::cout << "u " << u << " visible bit-rate: " << v_b[u][r] << std::endl;
              m_pav->HeatSelectedVersion(r);
              optiVisibleB.push_back(v_b[u][r]);
            }
          }
          std::cout << "-------" << std::endl;
        }
      }
      std::uniform_int_distribution<unsigned int> dist(0, offeredVersion.size()-1);
      for (unsigned u = 0; u < nbUser; ++u)
      {
        unsigned int randomR = dist(engine);
        randomVisibleB.push_back(v_b[u][randomR]);
        auto startPosition = psi_vid->GetSegments()[u]->GetStartPosition();
        Float minDist = 100;
        unsigned rMin = nbVersion;
        for (auto r: offeredVersion)
        {
          auto dist = m_pav->GetAllowedVersionVector()[r].Distance(startPosition);
          if (minDist > dist)
          {
            minDist = dist;
            rMin = r;
          }
          // std::cout << "r " << r << " dist " << dist  << " start " << startPosition << std::endl;
        }
        // std::cout << "heuristic: " << rMin << " dist " << minDist << " -> "<< v_b[u][rMin] << std::endl;
        heuristicVisibleB.push_back(v_b[u][rMin]);
      }
      std::cout << "##############" << std::endl;


      cplex.end();
      model.end();
      env.end();
      ++testCount;
      }
      catch (IloException& e) {
        std::cerr << "Concert exception caught: " << e << std::endl;
      }
      catch (...) {
        std::cerr << "Unknown exception caught" << std::endl;
      }
    }
  }

  //Write statistic outputs
  std::ofstream ofs(outputDir+"/results.txt");
  std::sort(optiVisibleB.begin(), optiVisibleB.end());
  std::sort(heuristicVisibleB.begin(), heuristicVisibleB.end());
  std::sort(randomVisibleB.begin(), randomVisibleB.end());
  std::sort(versionSize.begin(), versionSize.end());
  ofs << "cdf optiVisibleB heuristicVisibleB randomVisibleB Sqer\n";
  for (double percentile = 0.0; percentile <= 100.0; percentile += 1)
  {
    ofs << percentile << " " << Percentile(optiVisibleB, percentile) << " " << Percentile(heuristicVisibleB, percentile)
        << " " << Percentile(randomVisibleB, percentile) << " " << Percentile(versionSize, percentile) << "\n";
  }

  ofs = std::ofstream(outputDir+"/results_avg.txt");
  double avgOpti = 0;
  double avgHeuri = 0;
  double avgRandom = 0;
  double avgSqer = 0;
  for (auto const& val: optiVisibleB) {avgOpti+=val;} avgOpti/optiVisibleB.size();
  for (auto const& val: heuristicVisibleB) {avgHeuri+=val;} avgHeuri/heuristicVisibleB.size();
  for (auto const& val: randomVisibleB) {avgRandom+=val;} avgRandom/randomVisibleB.size();
  for (auto const& val: versionSize) {avgSqer+=val;} avgSqer/versionSize.size();
  ofs << avgOpti << " " << avgHeuri << " " << avgRandom << " "  << avgSqer << std::endl;

  m_pav->WritePosHeatMap(outputDir+"/pos_results.txt");
  m_pav->WriteDimHeatMap(outputDir+"/dim_results.txt");

}
