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
  for (auto n = 0; n <= nbArea; ++n)
  {
    double Squer = (n*4*PI)/nbArea;
    std::tie(b_qer_val[n], b_out_val[n]) = GetSurfaceBitrateQerOut(Squer, m_conf);
  }

  std::cout << "Nb views = " << psi_vid->NbView() << std::endl;

  //Create variables
  IloNumVarArray b_qer(env, nbUser, 0, 4*PI, ILOFLOAT); //b_qer[u]
  IloNumVarArray b_out(env, nbUser, 0, 4*PI, ILOFLOAT); //b_out[u]
  IloNumVarArray b_qer_j(env, m_conf->nbQer, 0, 4*PI, ILOFLOAT); //b_qer[u]
  IloNumVarArray b_out_j(env, m_conf->nbQer, 0, 4*PI, ILOFLOAT); //b_out[u]

  VarMatrix q_qer(env, nbUser); // q_qer[u][a]
  for (unsigned u = 0; u < nbUser; ++u)
  {
    q_qer[u] = IloNumVarArray(env, nbArea, 0, 1, ILOINT);
  }

  VarMatrix created_r(env, m_conf->nbQer);
  for (unsigned j = 0; j < m_conf->nbQer; ++j)
  {
    created_r[j] = IloNumVarArray(env, nbArea, 0, 1, ILOINT);
  }


  std::cout << "Generate constraints" << std::endl;
  try
  {
  //1.a
  for (unsigned j = 0; j < m_conf->nbQer; ++j)
  {
    for (unsigned n = 0; n <= nbArea; ++n)
    {
      IloIfThen selectBitrate(env, IloSum(created_r[j]) == n, (b_qer_j[j] == b_qer_val[n]) && (b_out_j[j] == b_out_val[n]) );
      selectBitrate.setName((std::string("1a_")+std::to_string(j)+std::string("_")+std::to_string(n)).c_str());
      model.add(selectBitrate);
    }
  }
  for (unsigned u = 0; u < nbUser; ++u)
  {
    // for (unsigned a = 0; a < nbArea; ++a)
    // {
    //   sc += q_qer[u][a] * b_qer[u] + (1-q_qer[u][a]) * b_out[u];
    // }
    // //1.b c d e
    // IloOr or_bitrate(env);
    // for (unsigned n = 0; n <= nbArea; ++n)
    // {
    //   or_bitrate.add((b_qer[u] == b_qer_val[n]) && (b_out[u] == b_out_val[n]));
    // }
    // or_bitrate.setName((std::string("1bcde")).c_str());
    // model.add(or_bitrate);

    //1.f
    IloOr or_repr(env);
    for (unsigned j = 0; j < m_conf->nbQer; ++j)
    {
      IloAnd and_qualitySelection(env);
      for (unsigned a = 0; a < nbArea; ++a)
      {
        and_qualitySelection.add(q_qer[u][a] == created_r[j][a]);
      }
      and_qualitySelection.add(b_qer_j[j] == b_qer[u]);
      and_qualitySelection.add(b_out_j[j] == b_out[u]);
      or_repr.add(and_qualitySelection);
    }
    or_repr.setName((std::string("1f")).c_str());
    model.add(or_repr);
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
      sc += v[u][a] * (q_qer[u][a] * b_qer[u] + (1-q_qer[u][a]) * b_out[u]);
      sc2 += v[u][a];
    }
  }
  model.add(IloMaximize(env, sc/sc2));

  // try
  // {
    std::cout << "Ready to run" << std::endl;

    IloCplex cplex(model);
    cplex.setParam(IloCplex::Threads, 1);
    cplex.setParam(IloCplex::NodeFileInd,3);
    cplex.setParam(IloCplex::WorkDir,".");
    cplex.setParam(IloCplex::MemoryEmphasis,true);

    std::cout << "Start solving" << std::endl;

    cplex.solve();
  }
  catch (IloException& e) {
    std::cerr << "Concert exception caught: " << e << std::endl;
  }
  catch (...) {
    std::cerr << "Unknown exception caught" << std::endl;
  }

  std::cout << "Done" << std::endl;
}
