{
  ifstream ifs("Histo_0_0.txt");
  int binc;
  TH1D* hist = new TH1D("hist","Histo_0_0.txt",16384,0,16384);
  int i = 0;
  while (ifs.is_open() && !ifs.eof()){
    ifs >> binc;
    hist->SetBinContent(i,binc);
    i++;
  }

  ifstream ifs2("Histo_0_1.txt");
  TH1D * hist2 = new TH1D("hist2","Histo_0_1.txt",16384,0,16384);
  i = 0;
  while (ifs2.is_open() && !ifs2.eof()){
    ifs2 >> binc;
    hist2->SetBinContent(i,binc);
    i++;
  }

  TCanvas * canv = new TCanvas("c","",2400,1600);
  hist->SetLineColor(kBlue);
  hist->Draw();
  canv->Update();
  hist2->SetLineColor(kRed);
  hist2->Draw("same");
}
