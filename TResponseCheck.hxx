#ifndef TRESPONSECHECK_HH
#define TRESPONSECHECK_HH

class TResponseCheck : public TBaseCFG {
    
public:
    
    TResponseCheck(char* name, vector<TSequencerCommand*> *sequence, TDevice* dev);
    void CalculateStatus();
    
private:
    
    TDevice* fDevice;
    
};

#endif
