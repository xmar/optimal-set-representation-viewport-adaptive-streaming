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
        if (u >= nbUser) {break;}
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
        for(unsigned a = 0; a < nbArea; ++a)
        {
          q_b_qer[u][a].setName((std::string("q_b_qer[u=") +std::to_string(u)+ std::string("][a=") +std::to_string(a)+ std::string("]")).c_str());
          q_b_out[u][a].setName((std::string("q_b_out[u=") +std::to_string(u)+ std::string("][a=") +std::to_string(a)+ std::string("]")).c_str());
        }
      }
      IloNumVarArray b_qer_j(env, m_conf->nbQer, min_b_qer, max_b_qer, ILOFLOAT); //b_qer[u]
      IloNumVarArray b_out_j(env, m_conf->nbQer, min_b_out, max_b_out, ILOFLOAT); //b_out[u]
      for (unsigned j = 0; j < m_conf->nbQer; ++j)
      {
        b_qer_j[j].setName((std::string("b_qer_j[j=") +std::to_string(j)+ std::string("]")).c_str());
        b_out_j[j].setName((std::string("b_out_j[j=") +std::to_string(j)+ std::string("]")).c_str());
      }

      VarIntMatrix q_qer(env, nbUser); // q_qer[u][a]
      for (unsigned u = 0; u < nbUser; ++u)
      {
        q_qer[u] = IloIntVarArray(env, nbArea, 0, 1);
        for (unsigned a = 0; a < nbArea; ++a)
        {
          q_qer[u][a].setName((std::string("q_qer[u=") +std::to_string(u)+ std::string("][a=") +std::to_string(a)+ std::string("]")).c_str());
        }
      }
      VarIntMatrix s(env, nbUser); // q_qer[u][a]
      for (unsigned u = 0; u < nbUser; ++u)
      {
        s[u] = IloIntVarArray(env, m_conf->nbQer, 0, 1);
        for (unsigned j = 0; j < m_conf->nbQer; ++j)
        {
          s[u][j].setName((std::string("s[u=") +std::to_string(u)+ std::string("][j=") +std::to_string(j)+ std::string("]")).c_str());
        }
      }

      VarIntMatrix nb(env, m_conf->nbQer);
      for (unsigned j = 0; j < m_conf->nbQer; ++j)
      {
        nb[j] = IloIntVarArray(env, nbArea+1, 0, 1);
        for (unsigned n = 0; n <= nbArea; ++n)
        {
          nb[j][n].setName((std::string("nb[j=") +std::to_string(j)+ std::string("][n=") +std::to_string(n)+ std::string("]")).c_str());
        }
      }

      IloIntVarArray usedCreatedRep(env, m_conf->nbQer, 0, 1);
      VarIntMatrix created_r(env, m_conf->nbQer);
      for (unsigned j = 0; j < m_conf->nbQer; ++j)
      {
        created_r[j] = IloIntVarArray(env, nbArea, 0, 1);
        usedCreatedRep[j].setName((std::string("usedCreatedRep[j=") +std::to_string(j)+ std::string("]")).c_str());
        for (unsigned a = 0; a < nbArea; ++a)
        {
          created_r[j][a].setName((std::string("created_r[j=") +std::to_string(j)+ std::string("][a=") +std::to_string(a)+ std::string("]")).c_str());
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

      for (unsigned j = 0; j < m_conf->nbQer; ++j)
      {
        //1.d
        IloConstraint ic0 = IloSum(nb[j]) == 1;
        ic0.setName((std::string("1.d[j=")+std::to_string(j)+std::string("]")).c_str());
        model.add(ic0);
        IloExpr sc_qer(env);
        IloExpr sc_out(env);
        IloExpr sc_nb(env);
        for(unsigned n = 0; n <= nbArea; ++n)
        {
          sc_qer += nb[j][n] * b_qer_val[n];
          sc_out += nb[j][n] * b_out_val[n];
          sc_nb += IloNum(n)* nb[j][n];
        }
        //1.e
        IloConstraint ic1 = b_qer_j[j] == sc_qer;
        ic1.setName((std::string("1.e[j=")+std::to_string(j)+std::string("]")).c_str());
        model.add(ic1);
        //1.f
        IloConstraint ic2 = b_out_j[j] == sc_out;
        ic2.setName((std::string("1.f[j=")+std::to_string(j)+std::string("]")).c_str());
        model.add(ic2);
        //1.g
        IloConstraint ic3 = IloSum(created_r[j]) == sc_nb;
        ic3.setName((std::string("1.g[j=")+std::to_string(j)+std::string("]")).c_str());
        model.add(ic3);

        //usedRep
        IloExpr sc_used(env);
        for (unsigned u = 0; u < nbUser; ++u)
        {
          sc_used += s[u][j];
        }
        model.add(sc_used >= usedCreatedRep[j]);
        model.add(sc_used <= IloNum(nbUser)*usedCreatedRep[j]);
      }

      for (unsigned u = 0; u < nbUser; ++u)
      {
        //Set the value of q_b_qer[u][a] to the value of q_qer of the selected j, idem for q_b_out[u][a]
        IloAnd andSec(env);
        for (unsigned a = 0; a < nbArea; ++a)
        {
          // 1.ca  //IloIfThen(env, q_qer[u][a] == 0, q_b_qer[u][a] == 0);
          IloConstraint ic1 = q_b_qer[u][a] <= q_qer[u][a] * M;
          ic1.setName((std::string("1.ca[u=")+std::to_string(u)+std::string("][a=")+std::to_string(a)+std::string("]")).c_str());
          andSec.add(ic1);
          // 1.cb //IloIfThen(env, q_qer[u][a] == 1, q_b_out[u][a] == 0);
          IloConstraint ic2 = q_b_out[u][a] <= (1 - q_qer[u][a]) * M;
          ic2.setName((std::string("1.cb[u=")+std::to_string(u)+std::string("][a=")+std::to_string(a)+std::string("]")).c_str());
          andSec.add(ic2);
          IloExpr sc_qer(env);
          IloExpr sc_out(env);
          for (unsigned j = 0; j < m_conf->nbQer; ++j)
          {
            // 1.cd
            IloConstraint ic1 = q_b_qer[u][a] <= (1 - s[u][j]) * M + b_qer_j[j];
            IloConstraint ic2 = -(1 - s[u][j]) * M + b_qer_j[j] <= q_b_qer[u][a];
            ic1.setName((std::string("1.cd1[u=")+std::to_string(u)+std::string("][a=")+std::to_string(a)+std::string("][j=")+std::to_string(j)+std::string("]")).c_str());
            ic2.setName((std::string("1.cd2[u=")+std::to_string(u)+std::string("][a=")+std::to_string(a)+std::string("][j=")+std::to_string(j)+std::string("]")).c_str());
            andSec.add(ic1);
            // andSec.add(ic2);
            // 1.ce
            IloConstraint ic3 = q_b_out[u][a] <= (1 - s[u][j]) * M + b_out_j[j];
            IloConstraint ic4 = -(1 - s[u][j]) * M + b_out_j[j] <= q_b_qer[u][a];
            ic3.setName((std::string("1.ce1[u=")+std::to_string(u)+std::string("][a=")+std::to_string(a)+std::string("][j=")+std::to_string(j)+std::string("]")).c_str());
            ic4.setName((std::string("1.ce2[u=")+std::to_string(u)+std::string("][a=")+std::to_string(a)+std::string("][j=")+std::to_string(j)+std::string("]")).c_str());
            andSec.add(ic3);
            // andSec.add(ic4);
          }

          for (unsigned j = 0; j < m_conf->nbQer; ++j)
          {
            // 1.b
            IloConstraint ic1 = s[u][j] - 1 <= q_qer[u][a] - created_r[j][a];
            IloConstraint ic2 = q_qer[u][a] - created_r[j][a] <= 1 - s[u][j];
            ic1.setName((std::string("1.b1[u=")+std::to_string(u)+std::string("][a=")+std::to_string(a)+std::string("][j=")+std::to_string(j)+std::string("]")).c_str());
            ic2.setName((std::string("1.b2[u=")+std::to_string(u)+std::string("][a=")+std::to_string(a)+std::string("][j=")+std::to_string(j)+std::string("]")).c_str());
            andSec.add(ic1);
            andSec.add(ic2);
            // IloConstraint ic1 = IloIfThen(env, s[u][j] == 1, q_qer[u][a] == created_r[j][a]);
            // ic1.setName((std::string("1.b1[u=")+std::to_string(u)+std::string("][a=")+std::to_string(a)+std::string("][j=")+std::to_string(j)+std::string("]")).c_str());
            // andSec.add(ic1);

          }
        }
        model.add(andSec);
        // 1.a
        IloConstraint ic1 = IloSum(s[u]) == 1;
        ic1.setName((std::string("1.a[u=")+std::to_string(u)+std::string("]")).c_str());
        model.add(ic1);
      }

      //1.H
      for (unsigned j = 0; j < m_conf->nbQer-1; ++j)
      {
        IloExpr sc_j1(env);
        IloExpr sc_j2(env);
        for (unsigned a = 0; a < nbArea; ++a)
        {
          sc_j1 += std::pow(2, a)*created_r[j][a];
          sc_j2 += std::pow(2, a)*created_r[j+1][a];
        }
        IloConstraint ic1 =  1 <= sc_j2 - sc_j1 ;//+ std::pow(2, nbArea) * (1 - usedCreatedRep[j+1]);
        ic1.setName((std::string("1.h[j=")+std::to_string(j)+std::string("]")).c_str());
        // model.add(ic1);

        model.add(usedCreatedRep[j] >= usedCreatedRep[j+1]);
      }
      IloConstraint ic1 =  1 <= IloSum(usedCreatedRep) ;
      ic1.setName((std::string("1.i")).c_str());
      model.add(ic1);

      std::cout << "Generate objective" << std::endl;

      //Objective
      IloExpr sc(env);
      for (unsigned u = 0; u < nbUser; ++u)
      {
        IloExpr sc_tmp(env);
        IloExpr sc2(env);
        PrintProgresionBar( float(u)/nbUser,100, std::chrono::microseconds(0), std::chrono::microseconds(0), nbUser-u);
        for (unsigned a = 0; a < nbArea; ++a)
        {
          // sc += v[u][a] * (q_qer[u][a] * b_qer[u] + (1-q_qer[u][a]) * b_out[u]);
          sc_tmp += v[u][a] * (q_b_qer[u][a] + q_b_out[u][a]);
          sc2 += v[u][a];
        }
        sc += sc_tmp/(sc2*nbUser);
      }
      model.add(IloMaximize(env, sc));

      // try
      // {
        std::cout << std::endl << "Ready to run" << std::endl;

        IloCplex cplex(model);
        cplex.setParam(IloCplex::EpGap,0.03);
        // cplex.setParam(IloCplex::Threads, 1);
        // cplex.setParam(IloCplex::NodeFileInd,3);
        // cplex.setParam(IloCplex::WorkDir,".");
        // cplex.setParam(IloCplex::MemoryEmphasis,true);
        cplex.setParam(IloCplex::DataCheck, true);

        cplex.exportModel((videoId+"_"+segmentId+"_model.lp").c_str());
        cplex.exportModel((videoId+"_"+segmentId+"_model.mps").c_str());

        std::cout << "Start solving" << std::endl;

        cplex.solve();

        std::cout << "Done" << std::endl;

        cplex.writeSolution((videoId+"_"+segmentId+"_solution.xsl").c_str());

        env.out() << cplex.getStatus() << std::endl;
        env.out() << cplex.getObjValue() << std::endl;

        std::cout << "====== Solution ======" << std::endl;
        for (unsigned uid = 0; uid < 11; ++uid)
        {
          IloNumArray s_sol(env);
          cplex.getValues(s_sol, s[uid]);
          unsigned rep = 0;
          std::cout << "User " << uid << " selected representation ";
          for (unsigned j = 0; j < m_conf->nbQer; ++j)
          {
            if (s_sol[j] > 0)
            {
              rep = j;
              std::cout << j << " ";
            }
          }
          std::cout << std::endl;

          unsigned n = 0;
          for (unsigned a = 0; a < nbArea; ++a)
          {
            n += cplex.getValue(created_r[rep][a]);
          }
          std::cout << "n = " << n << " B_qer_val = "<< b_qer_val[n] << " B_out_val = "<< b_out_val[n] << std::endl;

          for (unsigned n = 0; n <= nbArea; ++n)
          {
            if (cplex.getValue(nb[rep][n]) > 0)
            {
              std::cout << "Nb = " << n << " B_qer_val = "<< b_qer_val[n] << " B_out_val = "<< b_out_val[n] << std::endl;
            }
          }

          std::cout << "With B_qer = " << cplex.getValue(b_qer_j[rep]) << " B_out = "
            <<  cplex.getValue(b_out_j[rep]) << std::endl;

          double visible = 0;
          double sumV = 0;

          std::cout << "[";
          for (unsigned j = 0; j < m_conf->nbQer; ++j)
          {
            std::cout << cplex.getValue(usedCreatedRep[j]) << ", ";
          }
          std::cout << "]" << std::endl;

          for (unsigned a = 0; a < nbArea; ++a)
          {
            // std::cout << "(" << a << ", " << s_sol[rep] << ", (" << cplex.getValue(created_r[rep][a]) << ", "
            //   << cplex.getValue(q_qer[uid][a]) << "), ("<< cplex.getValue(q_b_qer[uid][a])
            //   << ", " << cplex.getValue(q_b_out[uid][a]) << ")) ";
            visible += v[uid][a] * (cplex.getValue(q_b_qer[uid][a]) + cplex.getValue(q_b_out[uid][a]));
            sumV += v[uid][a];
          }
          std::cout << std::endl;
          std::cout << "Visible: " << visible/sumV << std::endl;
          std::cout << std::endl;
          std::cout << std::endl;
        }
        for (unsigned j = 0; j < m_conf->nbQer; ++j)
        {
          if (cplex.getValue(usedCreatedRep[j]) > 0.5)
          {
            for (unsigned a = 0; a < nbArea; ++a)
            {
              if (cplex.getValue(created_r[j][a]) > 0.5)
              {
                m_areaSet->AddUseAsQer(a);
              }
            }
          }
        }

        cplex.end();
        model.end();
        env.end();
      }
      catch (IloException& e) {
        std::cerr << "Concert exception caught: " << e << std::endl;
      }
      catch (...) {
        std::cerr << "Unknown exception caught" << std::endl;
      }
    }
  }

  std::cout << "Done" << std::endl;
}
