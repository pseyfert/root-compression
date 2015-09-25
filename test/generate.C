int generate() {
  TFile* orgfile = TFile::Open("org.root");
  TList* thelist = orgfile->GetListOfKeys();
  TObjLink* thelink = thelist->FirstLink();
  while (thelink) {
    if (orgfile->Get(thelink->GetObject()->GetName())->InheritsFrom("TTree")) {
      TTree* t;
      orgfile->GetObject(thelink->GetObject()->GetName(),t);
      t->MakeClass("parent");
      TFile* uncompressed = new TFile("uncompressed.root","recreate","",ROOT::CompressionSettings(ROOT::kZLIB,0));
      uncompressed->WriteTObject(t->CloneTree(-1));
      uncompressed->Close();
      delete uncompressed;
      uncompressed=NULL;
      for (int i = 1 ; i < 10 ; ++i) {
        TFile* pointer = new TFile(Form("zlib%d.root",i),"recreate","",ROOT::CompressionSettings(ROOT::kZLIB,i));
        pointer->WriteTObject(t->CloneTree(-1));
        pointer->Close();
        delete pointer;
      }
      for (int i = 1 ; i < 10 ; ++i) {
        TFile* pointer = new TFile(Form("lzma%d.root",i),"recreate","",ROOT::CompressionSettings(ROOT::kLZMA,i));
        pointer->WriteTObject(t->CloneTree(-1));
        pointer->Close();
        delete pointer;
      }
      return 0;
      break;
    }
    thelink = thelink->Next();
  }
  return 1;
}
