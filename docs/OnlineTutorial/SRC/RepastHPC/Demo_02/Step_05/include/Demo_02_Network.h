/* Demo_02_Network.h */

#ifndef DEMO_02_NETWORK
#define DEMO_02_NETWORK

#include "repast_hpc/SharedContext.h"
#include "repast_hpc/SharedNetwork.h"


/* Custom Network Components */
template<typename V>
class DemoModelCustomEdge : public repast::RepastEdge<V>{
private:
    int confidence;
    
public:
    DemoModelCustomEdge(){}
    DemoModelCustomEdge(V* source, V* target) : repast::RepastEdge<V>(source, target) {}
    DemoModelCustomEdge(V* source, V* target, double weight) : repast::RepastEdge<V>(source, target, weight) {}
    DemoModelCustomEdge(V* source, V* target, double weight, int confidence) : repast::RepastEdge<V>(source, target, weight), confidence(confidence) {}
    
    DemoModelCustomEdge(boost::shared_ptr<V> source, boost::shared_ptr<V> target) : repast::RepastEdge<V>(source, target) {}
    DemoModelCustomEdge(boost::shared_ptr<V> source, boost::shared_ptr<V> target, double weight) : repast::RepastEdge<V>(source, target, weight) {}
    DemoModelCustomEdge(boost::shared_ptr<V> source, boost::shared_ptr<V> target, double weight, int confidence) : repast::RepastEdge<V>(source, target, weight), confidence(confidence) {}
    
    
    int getConfidence(){ return confidence; }
    void setConfidence(int con){ confidence = con; }
    
};

/* Custom Edge Content */
template<typename V>
struct DemoModelCustomEdgeContent : public repast::RepastEdgeContent<V>{
    
    friend class boost::serialization::access;
    
public:
    int confidence;
    DemoModelCustomEdgeContent(){}
    DemoModelCustomEdgeContent(DemoModelCustomEdge<V>* edge): repast::RepastEdgeContent<V>(edge), confidence(edge->getConfidence()){}
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        repast::RepastEdgeContent<V>::serialize(ar, version);
        ar & confidence;
    }
    
};

/* Custome Edge Content Manager */
template<typename V>
class DemoModelCustomEdgeContentManager {
public:
    DemoModelCustomEdgeContentManager(){}
    virtual ~DemoModelCustomEdgeContentManager(){}
    DemoModelCustomEdge<V>* createEdge(DemoModelCustomEdgeContent<V>& content, repast::Context<V>* context){
        return new DemoModelCustomEdge<V>(context->getAgent(content.source), context->getAgent(content.target), content.weight, content.confidence);
    }
    DemoModelCustomEdgeContent<V>* provideEdgeContent(DemoModelCustomEdge<V>* edge){
        return new DemoModelCustomEdgeContent<V>(edge);
    }
};

#endif