#include <iostream>
#include <cmath>
using namespace std;

#include "TH1D.h"
#include "TH2D.h"
#include "TFile.h"
#include "TTree.h"
#include "TGraphAsymmErrors.h"
#include "TLegend.h"
#include "TCanvas.h"

#include "PlotHelper3.h"
#include "SetStyle.h"

#define PI 3.14159265358979323846264338327950288479716939937510
#define BINCOUNT 16

int main()
{
   SetThesisStyle();

   bool IsMC = true;

   double CBinEdge[5+1] = {0, 0.1, 0.3, 0.5, 0.8, 1.0};
   double PTBinEdge[6+1] = {120, 140, 160, 180, 200, 300, 500};

   double MassBinEdge[BINCOUNT+1] = {0.00, 0.02, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.10, 0.12, 0.14, 0.16, 0.18, 0.20, 0.23, 0.26, 0.30};

   double RMS[5] = {17.2, 12.8, 7.5, 3.1, 1.0};
   double BigRMS = 32.17;

   TFile FData("StraightAA6Dijet.root");
   TFile FSmear("ScaledResult13PP6Dijet.root");
   // TFile FData("AAData_NoCrossSection.root");
   // TFile FSmear("PPDataHightPTJet.root");

   TTree *TData = (TTree *)FData.Get("Tree");
   TTree *TSmear = (TTree *)FSmear.Get("OutputTree");

   double DataSDRecoDR[10], DataSDZG[10], DataSDMass[10], DataSDPT[10], DataNewJetPT, DataJetPT, DataJetEta, DataJetPhi, DataMCWeight = 1, DataCentrality;
   TData->SetBranchAddress("SDRecoDR", &DataSDRecoDR);
   TData->SetBranchAddress("SDZG", &DataSDZG);
   TData->SetBranchAddress("SDMass", &DataSDMass);
   TData->SetBranchAddress("SDPT", &DataSDPT);
   TData->SetBranchAddress("NewJetPT", &DataNewJetPT);
   TData->SetBranchAddress("JetPT", &DataJetPT);
   TData->SetBranchAddress("JetEta", &DataJetEta);
   TData->SetBranchAddress("JetPhi", &DataJetPhi);
   if(IsMC == true)
      TData->SetBranchAddress("MCWeight", &DataMCWeight);
   TData->SetBranchAddress("Centrality", &DataCentrality);

   double SmearSDRecoDR, SmearSDZG, SmearSDMass, SmearSDPT, SmearNewJetPT, SmearNewJetEta, SmearNewJetPhi, SmearMCWeight = 1, SmearPTInJet, SmearRho, SmearTotalPT;
   TSmear->SetBranchAddress("BestJetDR2", &SmearSDRecoDR);
   TSmear->SetBranchAddress("BestJetZG2", &SmearSDZG);
   TSmear->SetBranchAddress("BestJetSDMass2", &SmearSDMass);
   TSmear->SetBranchAddress("BestJetSDPT2", &SmearSDPT);
   TSmear->SetBranchAddress("BestJetPT", &SmearNewJetPT);
   TSmear->SetBranchAddress("BestJetEta", &SmearNewJetEta);
   TSmear->SetBranchAddress("BestJetPhi", &SmearNewJetPhi);
   if(IsMC == true)
      TSmear->SetBranchAddress("MCWeight", &SmearMCWeight);
   TSmear->SetBranchAddress("TotalPTInJet", &SmearPTInJet);
   TSmear->SetBranchAddress("Rho", &SmearRho);
   TSmear->SetBranchAddress("TotalPT", &SmearTotalPT);

   TFile FDataSystematics("CombinedSystematics_Data.root");
   TFile FSmearSystematics("CombinedSystematics_All.root");

   TFile OutputFile("Graphs.root", "RECREATE");

   int DataEntryCount = TData->GetEntries();
   int SmearEntryCount = TSmear->GetEntries();

   PdfFileHelper PdfFile("SanityCheck.pdf");
   PdfFile.AddTextPage("Sanity checks!");

   for(int iC = 0; iC < 5; iC++)
   {
      for(int iPT = 0; iPT < 6; iPT++)
      {
         cout << "Start running iC = " << iC << ", iPT = " << iPT << endl;

         PdfFile.AddTextPage(Form("C %.02f %.02f, PT %.0f %.0f", CBinEdge[iC], CBinEdge[iC+1], PTBinEdge[iPT], PTBinEdge[iPT+1]));

         TGraphAsymmErrors *DataError = (TGraphAsymmErrors *)FDataSystematics.Get(Form("GRatio_C%d_P%d", iC, iPT));
         TGraphAsymmErrors *SmearError = (TGraphAsymmErrors *)FSmearSystematics.Get(Form("GRatio_C%d_P%d", iC, iPT));

         // Step 1 - get groomed PT fraction and derive reweighting
         TH1D HDataGroomed("HDataGroomed", ";groomed jet pt fraction;", 50, 0, 1);
         TH1D HSmearGroomed("HSmearGroomed", ";groomed jet pt fraction;", 50, 0, 1);
         TH1D HRatioGroomed("HRatioGroomed", ";groomed jet pt fraction;ratio", 50, 0, 1);

         for(int iE = 0; iE < DataEntryCount; iE++)
         {
            TData->GetEntry(iE);

            if(DataJetPT < PTBinEdge[iPT] || DataJetPT >= PTBinEdge[iPT+1])
               continue;
            if(DataCentrality < CBinEdge[iC] || DataCentrality >= CBinEdge[iC+1])
               continue;
            if(fabs(DataJetEta) > 1.3)
               continue;
            if(DataMCWeight > 10000)
               continue;

            HDataGroomed.Fill(DataSDPT[7] / DataNewJetPT, DataMCWeight);
            HRatioGroomed.Fill(DataSDPT[7] / DataNewJetPT, DataMCWeight);
         }
         PdfFile.AddPlot(HDataGroomed);

         for(int iE = 0; iE < SmearEntryCount; iE++)
         {
            TSmear->GetEntry(iE);
            
            if(SmearNewJetPT < PTBinEdge[iPT] || SmearNewJetPT >= PTBinEdge[iPT+1])
               continue;
            if(fabs(SmearNewJetEta) > 1.3)
               continue;

            // double ExcessPT = SmearPTInJet - SmearRho * 0.4 * 0.4 * PI;
            double ExcessPT = (SmearTotalPT - SmearRho * 1.2 * 1.2 * PI) / 9;
            double SmearWeight = exp(-ExcessPT * ExcessPT / (2 * RMS[iC] * RMS[iC])) / exp(-ExcessPT * ExcessPT / (2 * BigRMS * BigRMS));

            HSmearGroomed.Fill(SmearSDPT / SmearNewJetPT, SmearMCWeight * SmearWeight);
         }
         PdfFile.AddPlot(HSmearGroomed);

         HRatioGroomed.Divide(&HSmearGroomed);
         PdfFile.AddPlot(HRatioGroomed);

         // Step 2 - let's look at PT spectrum to make sure we don't see weird surprises here

         // Step 3 - build the graphs!
         TGraphAsymmErrors GData, GSmear, GDataSys, GSmearSys;

         GData.SetNameTitle(Form("Data_%d_%d", iC, iPT), "");
         GDataSys.SetNameTitle(Form("DataSys_%d_%d", iC, iPT), "");
         GSmear.SetNameTitle(Form("Smear_%d_%d", iC, iPT), "");
         GSmearSys.SetNameTitle(Form("SmearSys_%d_%d", iC, iPT), "");

         GData.SetMarkerStyle(20);
         GSmear.SetMarkerStyle(20);
         GDataSys.SetMarkerStyle(20);
         GSmearSys.SetMarkerStyle(20);
         GData.SetMarkerSize(0.7);
         GSmear.SetMarkerSize(0.7);
         GDataSys.SetMarkerSize(0.7);
         GSmearSys.SetMarkerSize(0.7);

         double DataGrandTotal = 0;
         vector<int> DataN(BINCOUNT);
         vector<double> DataCount(BINCOUNT);
         vector<double> DataTotalX(BINCOUNT);
         vector<double> DataTotalY(BINCOUNT);
         vector<double> DataTotalY2(BINCOUNT);
         vector<double> DataTotalYUp(BINCOUNT);
         vector<double> DataTotalYDown(BINCOUNT);

         for(int iE = 0; iE < DataEntryCount; iE++)
         {
            TData->GetEntry(iE);

            if(DataJetPT < PTBinEdge[iPT] || DataJetPT >= PTBinEdge[iPT+1])
               continue;
            if(DataCentrality < CBinEdge[iC] || DataCentrality >= CBinEdge[iC+1])
               continue;
            if(fabs(DataJetEta) > 1.3)
               continue;
            if(DataSDRecoDR[7] < 0.1)
               continue;
            if(DataMCWeight > 10000)
               continue;

            double SDMassRatio = DataSDMass[7] / DataNewJetPT;
            int SysBin = SDMassRatio / (0.40 / 160);

            int iB = -1;
            for(int i = 0; i < BINCOUNT; i++)
            {
               if(MassBinEdge[i] < SDMassRatio && SDMassRatio <= MassBinEdge[i+1])
               {
                  iB = i;
                  break;
               }
            }
            if(iB < 0 || iB >= BINCOUNT)
               continue;

            double TotalWeight = DataMCWeight;
            double UpWeight = DataError->GetErrorYhigh(SysBin);
            double DownWeight = -DataError->GetErrorYlow(SysBin);

            DataGrandTotal = DataGrandTotal + TotalWeight;
            DataN[iB] = DataN[iB] + 1;
            DataCount[iB] = DataCount[iB] + TotalWeight;
            DataTotalX[iB] = DataTotalX[iB] + SDMassRatio * TotalWeight;
            DataTotalY[iB] = DataTotalY[iB] + TotalWeight;
            DataTotalY2[iB] = DataTotalY2[iB] + TotalWeight * TotalWeight;
            DataTotalYUp[iB] = DataTotalYUp[iB] + TotalWeight * pow(10, UpWeight);
            DataTotalYDown[iB] = DataTotalYDown[iB] + TotalWeight * pow(10, DownWeight);
         }

         for(int i = 0; i < BINCOUNT; i++)
         {
            double BinSize = MassBinEdge[i+1] - MassBinEdge[i];
            double X = DataTotalX[i] / DataCount[i];
            double Y = DataTotalY[i] / BinSize / DataGrandTotal;
            // double YRMS = sqrt((DataTotalY2[i] - DataTotalY[i] * DataTotalY[i] / DataN[i]) / DataN[i]) / DataGrandTotal / BinSize;
            double YRMS = sqrt(DataTotalY2[i]) / DataGrandTotal / BinSize;
            double YUp = DataTotalYUp[i] / BinSize / DataGrandTotal;
            double YDown = DataTotalYDown[i] / BinSize / DataGrandTotal;

            GData.SetPoint(i, X, Y);
            GData.SetPointError(i, X - MassBinEdge[i], MassBinEdge[i+1] - X, YRMS, YRMS);
            GDataSys.SetPoint(i, X, Y);
            GDataSys.SetPointError(i, X - MassBinEdge[i], MassBinEdge[i+1] - X, Y - YDown, YUp - Y);
         }
         PdfFile.AddPlot(GData, "ap");
         PdfFile.AddPlot(GDataSys, "ap");
         
         GData.Write();
         GDataSys.Write();
      
         double SmearGrandTotal = 0;
         vector<int> SmearN(BINCOUNT, 0);
         vector<double> SmearCount(BINCOUNT, 0);
         vector<double> SmearTotalX(BINCOUNT, 0);
         vector<double> SmearTotalY(BINCOUNT, 0);
         vector<double> SmearTotalY2(BINCOUNT, 0);
         vector<double> SmearTotalYUp(BINCOUNT, 0);
         vector<double> SmearTotalYDown(BINCOUNT, 0);

         for(int iE = 0; iE < SmearEntryCount; iE++)
         {
            TSmear->GetEntry(iE);

            if(SmearNewJetPT < PTBinEdge[iPT] || SmearNewJetPT >= PTBinEdge[iPT+1])
               continue;
            if(fabs(SmearNewJetEta) > 1.3)
               continue;
            if(SmearSDRecoDR < 0.1)
               continue;

            // double ExcessPT = SmearPTInJet - SmearRho * 0.4 * 0.4 * PI;
            double ExcessPT = (SmearTotalPT - SmearRho * 1.2 * 1.2 * PI) / 9;
            double SmearWeight = exp(-ExcessPT * ExcessPT / (2 * RMS[iC] * RMS[iC])) / exp(-ExcessPT * ExcessPT / (2 * BigRMS * BigRMS));

            double GroomedWeight = HRatioGroomed.GetBinContent(HRatioGroomed.FindBin(SmearSDPT / SmearNewJetPT));
            
            double SDMassRatio = SmearSDMass / SmearNewJetPT;
            int SysBin = SDMassRatio / (0.40 / 160);

            int iB = -1;
            for(int i = 0; i < BINCOUNT; i++)
            {
               if(MassBinEdge[i] < SDMassRatio && SDMassRatio <= MassBinEdge[i+1])
               {
                  iB = i;
                  break;
               }
            }
            if(iB < 0 || iB >= BINCOUNT)
               continue;

            double TotalWeight = SmearMCWeight * SmearWeight * GroomedWeight;
            // double TotalWeight = SmearMCWeight * SmearWeight;
            double UpWeight = SmearError->GetErrorYhigh(SysBin);
            double DownWeight = -SmearError->GetErrorYlow(SysBin);

            SmearGrandTotal = SmearGrandTotal + TotalWeight;
            SmearN[iB] = SmearN[iB] + 1;
            SmearCount[iB] = SmearCount[iB] + TotalWeight;
            SmearTotalX[iB] = SmearTotalX[iB] + SDMassRatio * TotalWeight;
            SmearTotalY[iB] = SmearTotalY[iB] + TotalWeight;
            SmearTotalY2[iB] = SmearTotalY2[iB] + TotalWeight * TotalWeight;
            SmearTotalYUp[iB] = SmearTotalYUp[iB] + TotalWeight * pow(10, UpWeight);
            SmearTotalYDown[iB] = SmearTotalYDown[iB] + TotalWeight * pow(10, DownWeight);
         }

         for(int i = 0; i < BINCOUNT; i++)
         {
            double BinSize = MassBinEdge[i+1] - MassBinEdge[i];
            double X = SmearTotalX[i] / SmearCount[i];
            double Y = SmearTotalY[i] / BinSize / SmearGrandTotal;
            // double YRMS = sqrt((SmearTotalY2[i] - SmearTotalY[i] * SmearTotalY[i] / SmearN[i]) / SmearN[i]) / SmearGrandTotal / BinSize;
            double YRMS = sqrt(SmearTotalY2[i]) / SmearGrandTotal / BinSize;
            double YUp = SmearTotalYUp[i] / BinSize / SmearGrandTotal;
            double YDown = SmearTotalYDown[i] / BinSize / SmearGrandTotal;

            GSmear.SetPoint(i, X, Y);
            GSmear.SetPointError(i, X - MassBinEdge[i], MassBinEdge[i+1] - X, YRMS, YRMS);
            GSmearSys.SetPoint(i, X, Y);
            GSmearSys.SetPointError(i, X - MassBinEdge[i], MassBinEdge[i+1] - X, Y - YDown, YUp - Y);
         }
         PdfFile.AddPlot(GSmear, "ap");
         PdfFile.AddPlot(GSmearSys, "ap");
         
         GSmear.Write();
         GSmearSys.Write();

         TLegend Legend(0.5, 0.8, 0.8, 0.5);
         Legend.SetTextFont(42);
         Legend.SetTextSize(0.035);
         Legend.SetFillStyle(0);
         Legend.AddEntry(&GData, "PbPb (stat.)", "pl");
         Legend.AddEntry(&GDataSys, "PbPb (sys.)", "pl");
         Legend.AddEntry(&GSmear, "smeared pp (stat.)", "pl");
         Legend.AddEntry(&GSmearSys, "smeared pp (sys.)", "pl");

         TCanvas C;

         GData.SetMarkerColor(kBlack);
         GData.SetLineColor(kBlue);
         GSmear.SetMarkerColor(kRed);
         GSmear.SetLineColor(kMagenta);
         GSmearSys.SetMarkerColor(kRed);
         GSmearSys.SetLineColor(kRed);

         TH2D HWorld("HWorld", ";Jet SD Mass / Jet PT;a.u.", 100, 0, 0.3, 100, 0, 25);
         HWorld.SetStats(0);
         HWorld.Draw();
         GDataSys.Draw("p");
         GData.Draw("p");
         GSmearSys.Draw("p");
         GSmear.Draw("p");
         Legend.Draw();

         PdfFile.AddCanvas(C);
         
         TH2D HWorld2("HWorld2", ";Jet SD Mass / Jet PT;a.u.", 100, 0, 0.3, 100, 0.01, 500);
         HWorld2.SetStats(0);
         HWorld2.Draw();
         GDataSys.Draw("p");
         GData.Draw("p");
         GSmearSys.Draw("p");
         GSmear.Draw("p");
         Legend.Draw();

         C.SetLogy();
         PdfFile.AddCanvas(C);
      }
   }

   PdfFile.AddTimeStampPage();
   PdfFile.Close();

   OutputFile.Close();

   FSmearSystematics.Close();
   FDataSystematics.Close();

   FSmear.Close();
   FData.Close();

   return 0;
}







