/**************************************************************************

    This is the device code. This file contains the child class where
    custom functionality can be added to the device. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "USRP_UHD.h"

PREPARE_LOGGING(USRP_UHD_i)

USRP_UHD_i::USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    USRP_UHD_base(devMgr_ior, id, lbl, sftwrPrfl)
{
    construct();
}

USRP_UHD_i::USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    USRP_UHD_base(devMgr_ior, id, lbl, sftwrPrfl, compDev)
{
    construct();
}

USRP_UHD_i::USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    USRP_UHD_base(devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
    construct();
}

USRP_UHD_i::USRP_UHD_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    USRP_UHD_base(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev)
{
    construct();
}

USRP_UHD_i::~USRP_UHD_i()
{
}


/***********************************************************************************************

    Basic functionality:

        The service function is called by the serviceThread object (of type ProcessThread).
        This call happens immediately after the previous call if the return value for
        the previous call was NORMAL.
        If the return value for the previous call was NOOP, then the serviceThread waits
        an amount of time defined in the serviceThread's constructor.
        
    SRI:
        To create a StreamSRI object, use the following code:
                std::string stream_id = "testStream";
                BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);

    Time:
        To create a PrecisionUTCTime object, use the following code:
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();

        
    Ports:

        Data is passed to the serviceFunction through the getPacket call (BULKIO only).
        The dataTransfer class is a port-specific class, so each port implementing the
        BULKIO interface will have its own type-specific dataTransfer.

        The argument to the getPacket function is a floating point number that specifies
        the time to wait in seconds. A zero value is non-blocking. A negative value
        is blocking.  Constants have been defined for these values, bulkio::Const::BLOCKING and
        bulkio::Const::NON_BLOCKING.

        Each received dataTransfer is owned by serviceFunction and *MUST* be
        explicitly deallocated.

        To send data using a BULKIO interface, a convenience interface has been added 
        that takes a std::vector as the data input

        NOTE: If you have a BULKIO dataSDDS port, you must manually call 
              "port->updateStats()" to update the port statistics when appropriate.

        Example:
            // this example assumes that the device has two ports:
            //  A provides (input) port of type bulkio::InShortPort called short_in
            //  A uses (output) port of type bulkio::OutFloatPort called float_out
            // The mapping between the port and the class is found
            // in the device base class header file

            bulkio::InShortPort::dataTransfer *tmp = short_in->getPacket(bulkio::Const::BLOCKING);
            if (not tmp) { // No data is available
                return NOOP;
            }

            std::vector<float> outputData;
            outputData.resize(tmp->dataBuffer.size());
            for (unsigned int i=0; i<tmp->dataBuffer.size(); i++) {
                outputData[i] = (float)tmp->dataBuffer[i];
            }

            // NOTE: You must make at least one valid pushSRI call
            if (tmp->sriChanged) {
                float_out->pushSRI(tmp->SRI);
            }
            float_out->pushPacket(outputData, tmp->T, tmp->EOS, tmp->streamID);

            delete tmp; // IMPORTANT: MUST RELEASE THE RECEIVED DATA BLOCK
            return NORMAL;

        If working with complex data (i.e., the "mode" on the SRI is set to
        true), the std::vector passed from/to BulkIO can be typecast to/from
        std::vector< std::complex<dataType> >.  For example, for short data:

            bulkio::InShortPort::dataTransfer *tmp = myInput->getPacket(bulkio::Const::BLOCKING);
            std::vector<std::complex<short> >* intermediate = (std::vector<std::complex<short> >*) &(tmp->dataBuffer);
            // do work here
            std::vector<short>* output = (std::vector<short>*) intermediate;
            myOutput->pushPacket(*output, tmp->T, tmp->EOS, tmp->streamID);

        Interactions with non-BULKIO ports are left up to the device developer's discretion

    Properties:
        
        Properties are accessed directly as member variables. For example, if the
        property name is "baudRate", it may be accessed within member functions as
        "baudRate". Unnamed properties are given a generated name of the form
        "prop_n", where "n" is the ordinal number of the property in the PRF file.
        Property types are mapped to the nearest C++ type, (e.g. "string" becomes
        "std::string"). All generated properties are declared in the base class
        (USRP_UHD_base).
    
        Simple sequence properties are mapped to "std::vector" of the simple type.
        Struct properties, if used, are mapped to C++ structs defined in the
        generated file "struct_props.h". Field names are taken from the name in
        the properties file; if no name is given, a generated name of the form
        "field_n" is used, where "n" is the ordinal number of the field.
        
        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            //  - A boolean called scaleInput
              
            if (scaleInput) {
                dataOut[i] = dataIn[i] * scaleValue;
            } else {
                dataOut[i] = dataIn[i];
            }
            
        Callback methods can be associated with a property so that the methods are
        called each time the property value changes.  This is done by calling 
        addPropertyChangeListener(<property name>, this, &USRP_UHD_i::<callback method>)
        in the constructor.

        Callback methods should take two arguments, both const pointers to the value
        type (e.g., "const float *"), and return void.

        Example:
            // This example makes use of the following Properties:
            //  - A float value called scaleValue
            
        //Add to USRP_UHD.cpp
        USRP_UHD_i::USRP_UHD_i(const char *uuid, const char *label) :
            USRP_UHD_base(uuid, label)
        {
            addPropertyChangeListener("scaleValue", this, &USRP_UHD_i::scaleChanged);
        }

        void USRP_UHD_i::scaleChanged(const float *oldValue, const float *newValue)
        {
            std::cout << "scaleValue changed from" << *oldValue << " to " << *newValue
                      << std::endl;
        }
            
        //Add to USRP_UHD.h
        void scaleChanged(const float* oldValue, const float* newValue);
        
        
************************************************************************************************/
int USRP_UHD_i::serviceFunction()
{
    LOG_DEBUG(USRP_UHD_i, "serviceFunction() example log message");
    
    return NOOP;
}

void USRP_UHD_i::construct()
{
    USRP_UHD_base::construct();
    /***********************************************************************************
     this function is invoked in the constructor
    ***********************************************************************************/
}
/*************************************************************
Functions supporting tuning allocation
*************************************************************/
void USRP_UHD_i::deviceEnable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to set the 'enabled' member of fts to indicate that tuner as enabled
    ************************************************************/
    #warning deviceEnable(): Enable the given tuner  *********
    return;
}
void USRP_UHD_i::deviceDisable(frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
    ************************************************************/
    #warning deviceDisable(): Disable the given tuner  *********
    return;
}
bool USRP_UHD_i::deviceSetTuning(const frontend::frontend_tuner_allocation_struct &request, frontend_tuner_status_struct_struct &fts, size_t tuner_id){
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    return true if the tuning succeeded, and false if it failed
    ************************************************************/
    #warning deviceSetTuning(): Evaluate whether or not a tuner is added  *********
    return BOOL_VALUE_HERE;
}
bool USRP_UHD_i::deviceDeleteTuning(frontend_tuner_status_struct_struct &fts, size_t tuner_id) {
    /************************************************************
    modify fts, which corresponds to this->frontend_tuner_status[tuner_id]
    return true if the tune deletion succeeded, and false if it failed
    ************************************************************/
    #warning deviceDeleteTuning(): Deallocate an allocated tuner  *********
    return BOOL_VALUE_HERE;
}

/*************************************************************
Functions servicing the RFInfo port(s)
- port_name is the port over which the call was received
*************************************************************/
std::string USRP_UHD_i::get_rf_flow_id(const std::string& port_name){return std::string("none");}
void USRP_UHD_i::set_rf_flow_id(const std::string& port_name, const std::string& id){}
frontend::RFInfoPkt USRP_UHD_i::get_rfinfo_pkt(const std::string& port_name){frontend::RFInfoPkt tmp;return tmp;}
void USRP_UHD_i::set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt &pkt){}
/*************************************************************
Functions servicing the tuner control port
*************************************************************/
std::string USRP_UHD_i::getTunerType(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    return frontend_tuner_status[idx].tuner_type;
}
bool USRP_UHD_i::getTunerDeviceControl(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    if (getControlAllocationId(idx) == allocation_id)
        return true;
    return false;
}
std::string USRP_UHD_i::getTunerGroupId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    return frontend_tuner_status[idx].group_id;
}
std::string USRP_UHD_i::getTunerRfFlowId(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    return frontend_tuner_status[idx].rf_flow_id;
}
void USRP_UHD_i::setTunerCenterFrequency(const std::string& allocation_id, double freq) {
    long idx = getTunerMapping(allocation_id);
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    if (freq<0) throw FRONTEND::BadParameterException();
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].center_frequency = freq;
}
double USRP_UHD_i::getTunerCenterFrequency(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    return frontend_tuner_status[idx].center_frequency;
}
void USRP_UHD_i::setTunerBandwidth(const std::string& allocation_id, double bw) {
    long idx = getTunerMapping(allocation_id);
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    if (bw<0) throw FRONTEND::BadParameterException();
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].bandwidth = bw;
}
double USRP_UHD_i::getTunerBandwidth(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    return frontend_tuner_status[idx].bandwidth;
}
void USRP_UHD_i::setTunerAgcEnable(const std::string& allocation_id, bool enable){throw FRONTEND::NotSupportedException("setTunerAgcEnable not supported");}
bool USRP_UHD_i::getTunerAgcEnable(const std::string& allocation_id){throw FRONTEND::NotSupportedException("getTunerAgcEnable not supported");return false;}
void USRP_UHD_i::setTunerGain(const std::string& allocation_id, float gain){throw FRONTEND::NotSupportedException("setTunerGain not supported");}
float USRP_UHD_i::getTunerGain(const std::string& allocation_id){throw FRONTEND::NotSupportedException("getTunerGain not supported");return 0.0;}
void USRP_UHD_i::setTunerReferenceSource(const std::string& allocation_id, long source){throw FRONTEND::NotSupportedException("setTunerReferenceSource not supported");}
long USRP_UHD_i::getTunerReferenceSource(const std::string& allocation_id){throw FRONTEND::NotSupportedException("getTunerReferenceSource not supported");return 0;}
void USRP_UHD_i::setTunerEnable(const std::string& allocation_id, bool enable) {
    long idx = getTunerMapping(allocation_id);
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].enabled = enable;
}
bool USRP_UHD_i::getTunerEnable(const std::string& allocation_id) {
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    return frontend_tuner_status[idx].enabled;
}

void USRP_UHD_i::setTunerOutputSampleRate(const std::string& allocation_id, double sr) {
    long idx = getTunerMapping(allocation_id);
    if(allocation_id != getControlAllocationId(idx))
        throw FRONTEND::FrontendException(("ID "+allocation_id+" does not have authorization to modify the tuner").c_str());
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    if (sr<0) throw FRONTEND::BadParameterException();
    // set hardware to new value. Raise an exception if it's not possible
    this->frontend_tuner_status[idx].sample_rate = sr;
}
double USRP_UHD_i::getTunerOutputSampleRate(const std::string& allocation_id){
    long idx = getTunerMapping(allocation_id);
    if (idx < 0) throw FRONTEND::BadParameterException("Invalid allocation id");
    return frontend_tuner_status[idx].sample_rate;
}

