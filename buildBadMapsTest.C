void buildBadMapTest(const char* file1="BadMap.root",
                     const char* file2="PedestalTable_269924.root")
{
    TFile *_file0 = TFile::Open(file1);
    TFile *_file1 = TFile::Open(file2);

    TH2I* h1 = (TH2I*)_file0->Get("PHOS_BadMap_mod1");
    TH2I* h2 = (TH2I*)_file0->Get("PHOS_BadMap_mod2");
    TH2I* h3 = (TH2I*)_file0->Get("PHOS_BadMap_mod3");
    TH2I* h4 = (TH2I*)_file0->Get("PHOS_BadMap_mod4");
    
    TH2F* hPed1 = (TH2F*)_file1->Get("hPedHiMeanm1");
    TH2F* hPed2 = (TH2F*)_file1->Get("hPedHiMeanm2");
    TH2F* hPed3 = (TH2F*)_file1->Get("hPedHiMeanm3");
    TH2F* hPed4 = (TH2F*)_file1->Get("hPedHiMeanm4");

    h1->Add(hPed1);
    h2->Add(hPed2);
    h3->Add(hPed3);
    h4->Add(hPed4);
    
    TCanvas* cc = new TCanvas("cc","",1108,824); // WindowWidth, WindowHeight
    cc->Divide(2,2);
    
    gStyle->SetOptStat(0);
    gStyle->SetOptLogz();
    
    Double_t zmin = 0.;
    Double_t zmax = 20.;
    
    cc->cd(1);
    h1->Draw("col");
    h1->GetZaxis()->SetRangeUser(zmin,zmax);
    //gPad->SetLogz(1);
    gPad->SetGridx();
    gPad->SetGridy();

    cc->cd(2);
    h2->Draw("col");
    h2->GetZaxis()->SetRangeUser(zmin,zmax);
    //gPad->SetLogz(1);
    gPad->SetGridx();
    gPad->SetGridy();

    cc->cd(3);
    h3->Draw("col");
    h3->GetZaxis()->SetRangeUser(zmin,zmax);
    //gPad->SetLogz(1);
    gPad->SetGridx();
    gPad->SetGridy();

    cc->cd(4);
    h4->Draw("col");
    h4->GetZaxis()->SetRangeUser(zmin,zmax);
    //gPad->SetLogz(1);
    gPad->SetGridx();
    gPad->SetGridy();

}
