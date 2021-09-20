
void Ana()
{
    gStyle->SetOptStat("ouRMe");
    ifstream T2in;
    T2in.open("/Users/guihonghuang/base-7.0.6/extensions/src/marchive/log.txt", ios::in);
    ifstream T1in;
    T1in.open("/Users/guihonghuang/base-7.0.6/extensions/src/mcaput/putlog.txt", ios::in);

    TH1D* hT = new TH1D("","",200,0,800);
    hT->GetXaxis()->SetTitle("time [ns]");
    hT->GetYaxis()->SetTitle("count");
    hT->SetLineColor(4);
    hT->SetLineWidth(2);
    double t1;
    double t2;
    double temp;
    string stemp;

    for(int i=0;i<500;i++) {
        T2in>>stemp;
        T2in>>t1;
        T2in>>temp;
        T2in>>t2;
        T2in>>temp;

        //T1in>>temp;
        //T1in>>t1;
        //T1in>>temp;
        //t1 = (t1/1000000. - int(t1/1000000.))*1000000.;

        hT->Fill(t2-t1);
    }
    hT->Draw();
}




