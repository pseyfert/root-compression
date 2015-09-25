int generate() {
  TFile* orgfile = TFile::Open("org.root");
  TFile* uncompressed = new TFile("uncompressed.root","recreate","",ROOT::CompressionSettings(ROOT::kZLIB,0));
  vector<TFile*> zlibs;
  for (int i = 1; i < 10 ; ++i) zlibs.push_back(new TFile(Form("zlib%d.root",i),"recreate","",ROOT::CompressionSettings(ROOT::kZLIB,i)));
  vector<TFile*> lzmas;
  for (int i = 1; i < 10 ; ++i) lzmas.push_back(new TFile(Form("lzma%d.root",i),"recreate","",ROOT::CompressionSettings(ROOT::kLZMA,i)));
  TList* thelist = orgfile->GetListOfKeys();
  TObjLink* thelink = thelist->FirstLink();
  while (thelink) {
    if (orgfile->Get(thelink->GetObject()->GetName())->InheritsFrom("TTree")) {
      TTree* t;
      orgfile->GetObject(thelink->GetObject()->GetName(),t);
      t->MakeClass("parent");
      uncompressed->WriteTObject(t->CloneTree(-1),0,"WriteDelete");
      uncompressed->Close();
      delete uncompressed;
      uncompressed=NULL;
      for (auto pointer : zlibs) {
        pointer->WriteTObject(t->CloneTree(-1),0,"WriteDelete");
        pointer->Close();
        delete pointer;
        pointer=NULL;
      }
      for (auto pointer : lzmas) {
        pointer->WriteTObject(t->CloneTree(-1),0,"WriteDelete");
        pointer->Close();
        delete pointer;
        pointer=NULL;
      }
      return 0;
      break;
    }
    thelink = thelink->Next();
  }
  return 1;
}
