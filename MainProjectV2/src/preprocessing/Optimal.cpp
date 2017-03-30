#include "Optimal.hpp"

#include "common/ConfigArgs.hpp"
#include "common/Common.hpp"
#include "AreaSet.hpp"
#include "PrecomputeSegmentsIntersections.hpp"

#include <ilcplex/ilocplex.h>

#include <iostream>

using namespace IMT;

typedef IloArray<IloNumArray> FloatMatrix;
typedef IloArray<IloNumVarArray> VarMatrix;

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

  IloEnv env;
  IloModel model(env);

  unsigned nbArea = m_areaSet->GetAreas().size();
  auto psi_vid = m_psi->FilterVidSegId("bmx", "0");
  unsigned nbUser = psi_vid->GetSegments().size();

  //const values:
  IloNumArray b_qer_val(env, nbArea+1); //b_qer[n]
  IloNumArray b_out_val(env, nbArea+1); //b_out[n]
  FloatMatrix v(env, nbUser); //v[u][a]
  for (unsigned u = 0; u < nbUser; ++u)
  {
    v[u] = IloNumArray(env, nbArea);
    for (unsigned a = 0; a < nbArea; ++a)
    {
      v[u][a] = 0;
    }
  }
  std::cout << "Nb user = " << nbUser << std::endl;
  unsigned u = 0;
  unsigned t = 0;
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
  }
  double max_b_qer(0.0);
  double max_b_out(0.0);
  double min_b_qer(4*PI);
  double min_b_out(4*PI);
  for (auto n = 0; n <= nbArea; ++n)
  {
    double Squer = (n*4*PI)/nbArea;
    std::tie(b_qer_val[n], b_out_val[n]) = GetSurfaceBitrateQerOut(Squer, m_conf);
    max_b_qer = std::max(max_b_qer, b_qer_val[n]);
    max_b_out = std::max(max_b_out, b_out_val[n]);
    min_b_qer = std::min(min_b_qer, b_qer_val[n]);
    min_b_out = std::min(min_b_out, b_out_val[n]);
  }

  std::cout << "Nb views = " << psi_vid->NbView() << std::endl;

  //Create variables
  VarMatrix q_b_qer(env, nbUser); //q_b_qer[u][a]
  VarMatrix q_b_out(env, nbUser); //q_b_out[u][a]
  for (unsigned u = 0; u  < nbUser; ++u)
  {
    q_b_qer[u] = IloNumVarArray(env, nbArea, 0, max_b_qer, ILOFLOAT);
    q_b_out[u] = IloNumVarArray(env, nbArea, 0, max_b_qer, ILOFLOAT);
  }
  IloNumVarArray b_qer_j(env, m_conf->nbQer, min_b_qer, max_b_qer, ILOFLOAT); //b_qer[u]
  IloNumVarArray b_out_j(env, m_conf->nbQer, min_b_out, max_b_out, ILOFLOAT); //b_out[u]


  VarMatrix q_qer(env, nbUser); // q_qer[u][a]
  for (unsigned u = 0; u < nbUser; ++u)
  {
    q_qer[u] = IloNumVarArray(env, nbArea, 0, 1, ILOINT);
    for (unsigned a = 0; a < nbArea; ++a)
    {
      q_qer[u][a].setName((std::string("q_qer[") +std::to_string(u)+ std::string("][") +std::to_string(a)+ std::string("]")).c_str());
    }
  }
  VarMatrix s(env, nbUser); // q_qer[u][a]
  VarMatrix d(env, nbUser);
  for (unsigned u = 0; u < nbUser; ++u)
  {
    s[u] = IloNumVarArray(env, m_conf->nbQer, 0, 1, ILOINT);
    d[u] = IloNumVarArray(env, m_conf->nbQer, 0, 1, ILOINT);
    for (unsigned j = 0; j < m_conf->nbQer; ++j)
    {
      s[u][j].setName((std::string("s[") +std::to_string(u)+ std::string("][") +std::to_string(j)+ std::string("]")).c_str());
    }
  }

  VarMatrix nb(env, m_conf->nbQer);
  for (unsigned j = 0; j < m_conf->nbQer; ++j)
  {
    nb[j] = IloNumVarArray(env, nbArea+1, 0, 1, ILOINT);
    for (unsigned n = 0; n <= nbArea; ++n)
    {
      nb[j][n].setName((std::string("nb[") +std::to_string(j)+ std::string("][") +std::to_string(n)+ std::string("]")).c_str());
    }
  }

  VarMatrix created_r(env, m_conf->nbQer);
  for (unsigned j = 0; j < m_conf->nbQer; ++j)
  {
    created_r[j] = IloNumVarArray(env, nbArea, 0, 1, ILOINT);
    for (unsigned a = 0; a < nbArea; ++a)
    {
      created_r[j][a].setName((std::string("created_r[") +std::to_string(j)+ std::string("][") +std::to_string(a)+ std::string("]")).c_str());
    }
  }


  std::cout << "Generate constraints" << std::endl;
  try{
  // for (unsigned j = 0; j < m_conf->nbQer; ++j)
  // {
  //   for (unsigned n = 0; n <= nbArea; ++n)
  //   {
  //     IloIfThen selectBitrate(env, IloSum(created_r[j]) == n, (b_qer_j[j] == b_qer_val[n]) && (b_out_j[j] == b_out_val[n]) );
  //     selectBitrate.setName((std::string("1a_")+std::to_string(j)+std::string("_")+std::to_string(n)).c_str());
  //     model.add(selectBitrate);
  //   }
  // }

  // //set tile constraints
  // unsigned nbHTile = 4;
  // unsigned nbVTile = 4;
  // for (unsigned thetaTile = 0; thetaTile < nbHTile; ++thetaTile)
  // {
  //   for (unsigned phiTile = 0; phiTile < nbVTile; ++phiTile)
  //   {
  //     auto areaIds = m_areaSet->GetAreaIdInTile(thetaTile*2*PI/nbHTile - PI, (thetaTile+1)*2*PI/nbHTile - PI,
  //                                               phiTile*PI/nbVTile - PI, (phiTile+1)*PI/nbVTile);
  //     if (areaIds.size() > 1)
  //     {
  //       for (unsigned j = 0; j < m_conf->nbQer; ++j)
  //       {
  //         IloAnd tileContraint(env);
  //         for (unsigned i = 1; i < areaIds.size(); ++i)
  //         {
  //           tileContraint.add(created_r[j][areaIds[0]] == created_r[j][areaIds[i]]);
  //         }
  //         tileContraint.setName((std::string("tile")+std::to_string(thetaTile)+std::string("_")+std::to_string(phiTile)).c_str());
  //         model.add(tileContraint);
  //       }
  //     }
  //   }
  // }

  double M = 4*PI;
  double e = 0.0000001;

  for (unsigned u = 0; u < nbUser; ++u)
  {
    //Set the value of q_b_qer[u][a] to the value of q_qer of the selected j, idem for q_b_out[u][a]
    IloAnd andSec(env);
    for (unsigned a = 0; a < nbArea; ++a)
    {
      // 1.ca
      andSec.add(q_b_qer[u][a] <= q_qer[u][a] * M);
      // 1.cb
      andSec.add(q_b_out[u][a] <= (1 - q_qer[u][a]) * M);
      IloExpr sc_qer(env);
      IloExpr sc_out(env);
      for (unsigned j = 0; j < m_conf->nbQer; ++j)
      {
        // 1.cd
        andSec.add(q_b_qer[u][a] <= s[u][j] * M + b_qer_j[j]);
        // 1.ce
        andSec.add(q_b_out[u][a] <= s[u][j] * M + b_out_j[j]);
      }

      for (unsigned j = 0; j < m_conf->nbQer; ++j)
      {
        // 1.b
        andSec.add( s[u][j] - 1 <= q_qer[u][a] - created_r[j][a] <= 1 - s[u][j] );
      }
    }
    IloConstraint ic = andSec;
    ic.setName((std::string("1.c_1.b[")+std::to_string(u)+std::string("]")).c_str());
    model.add(andSec);
    // 1.a
    model.add(IloSum(s[u]) == 1);
  }

  for (unsigned j = 0; j < m_conf->nbQer; ++j)
  {
    //1.d
    model.add(IloSum(nb[j]) == 1);
    IloExpr sc_qer(env);
    IloExpr sc_out(env);
    for(unsigned n = 0; n <= nbArea; ++n)
    {
      sc_qer += nb[j][n] * b_qer_val[n];
      sc_out += nb[j][n] * b_out_val[n];
    }
    //1.e
    model.add(b_qer_j[j] == sc_qer);
    //1.f
    model.add(b_out_j[j] == sc_out);
  }

  std::cout << "Generate objective" << std::endl;

  //Objective
  IloExpr sc(env);
  IloExpr sc2(env);
  for (unsigned u = 0; u < nbUser; ++u)
  {
    PrintProgresionBar( float(u)/nbUser,100, std::chrono::microseconds(0), std::chrono::microseconds(0), nbUser-u);
    for (unsigned a = 0; a < nbArea; ++a)
    {
      // sc += v[u][a] * (q_qer[u][a] * b_qer[u] + (1-q_qer[u][a]) * b_out[u]);
      sc += v[u][a] * (q_b_qer[u][a] + q_b_out[u][a]);
      sc2 += v[u][a];
    }
  }
  model.add(IloMaximize(env, sc/sc2));

  // try
  // {
    std::cout << std::endl << "Ready to run" << std::endl;

    IloCplex cplex(model);
    cplex.setParam(IloCplex::Threads, 1);
    // cplex.setParam(IloCplex::NodeFileInd,3);
    // cplex.setParam(IloCplex::WorkDir,".");
    // cplex.setParam(IloCplex::MemoryEmphasis,true);
    cplex.setParam(IloCplex::DataCheck, true);

    cplex.exportModel("model.lp");
    cplex.exportModel("model.mps");

    std::cout << "Start solving" << std::endl;

    cplex.solve();

    std::cout << "Done" << std::endl;

    env.out() << cplex.getStatus() << std::endl;
    env.out() << cplex.getObjValue() << std::endl;
  }
  catch (IloException& e) {
    std::cerr << "Concert exception caught: " << e << std::endl;
  }
  catch (...) {
    std::cerr << "Unknown exception caught" << std::endl;
  }

  std::cout << "Done" << std::endl;
}
