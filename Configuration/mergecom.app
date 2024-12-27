[MERGE_STORE_SCP]
    PORT_NUMBER     = 104
    HOST_NAME       = localhost
    SERVICE_LIST  = Storage_SCU_Service_List

[SCP_AE]
    PORT_NUMBER     = 5010             # port 104 is the standard DICOM port and is privileged                                  HOST_NAME       = localhost 	# Put the remote machine name here
    SERVICE_LIST    = Query_SCU_Service_List
    
[MWL_SCP]
    PORT_NUMBER     = 220
    HOST_NAME       = localhost
    SERVICE_LIST    = Simulator_Worklist_Service_List

[Connectivity_SCP_Service_List]

    SERVICES_SUPPORTED       = 4 
   
    SERVICE_1                = STORAGE_COMMITMENT_PUSH
    ROLE_1                   = SCU

    SERVICE_2                = STANDARD_XRAY_ANGIO  
    SYNTAX_LIST_2            = Full_Syntax_List
    
    SERVICE_3                = STANDARD_GRAYSCALE_SOFTCOPY_PS
    SYNTAX_LIST_3            = ELE_ILE_EBE_Syntax_List
	
	SERVICE_4                = STANDARD_SEC_CAPTURE
    SYNTAX_LIST_4            = Full_Syntax_List 


[Query_SCU_Service_List]
    SERVICES_SUPPORTED      = 4     
    SERVICE_1               = STUDY_ROOT_QR_FIND
    SYNTAX_LIST_1           = ELE_ILE_EBE_Syntax_List

    SERVICE_2               = STUDY_ROOT_QR_MOVE
    SYNTAX_LIST_2           = ELE_ILE_EBE_Syntax_List

    SERVICE_3               = PATIENT_ROOT_QR_FIND
    SYNTAX_LIST_3           = ELE_ILE_EBE_Syntax_List

    SERVICE_4               = PATIENT_ROOT_QR_MOVE
    SYNTAX_LIST_4           = ELE_ILE_EBE_Syntax_List

[ELE_ILE_Syntax_List]
    SYNTAXES_SUPPORTED      = 2
    SYNTAX_1                = EXPLICIT_LITTLE_ENDIAN
    SYNTAX_2                = IMPLICIT_LITTLE_ENDIAN

[RDSR_SCU_Service_List]
    SERVICES_SUPPORTED       = 1
    MAX_OPERATIONS_INVOKED   = 1
    MAX_OPERATIONS_PERFORMED = 1

    SERVICE_1                = XRAY_RADIATION_DOSE_SR  
    SYNTAX_LIST_1            = ELE_ILE_EBE_Syntax_List


[Print_Service_List]
	SERVICES_SUPPORTED       = 2
    MAX_OPERATIONS_INVOKED   = 1
    MAX_OPERATIONS_PERFORMED = 1

    SERVICE_1                = BASIC_GRAYSCALE_PRINT_MANAGEMENT
    SYNTAX_LIST_1            = ELE_ILE_EBE_Syntax_List

    SERVICE_2                = BASIC_COLOR_PRINT_MANAGEMENT
    SYNTAX_LIST_2            = ELE_ILE_EBE_Syntax_List


[Storage_SCU_Service_List]
    SERVICES_SUPPORTED       = 4
    MAX_OPERATIONS_INVOKED   = 1
    MAX_OPERATIONS_PERFORMED = 1

    SERVICE_1                = STANDARD_XRAY_ANGIO
    SYNTAX_LIST_1            = ELE_ILE_EBE_Syntax_List

    SERVICE_2                = STANDARD_GRAYSCALE_SOFTCOPY_PS
    SYNTAX_LIST_2            = ELE_ILE_EBE_Syntax_List
	
    SERVICE_3                = XRAY_RADIATION_DOSE_SR
    SYNTAX_LIST_3            = ELE_ILE_EBE_Syntax_List

    SERVICE_4               = STANDARD_SEC_CAPTURE
    SYNTAX_LIST_4            = ELE_ILE_EBE_Syntax_List
  

[Storage_SCU_Service_List_JPEG]
    SERVICES_SUPPORTED       = 4
    MAX_OPERATIONS_INVOKED   = 1
    MAX_OPERATIONS_PERFORMED = 1

    SERVICE_1                = STANDARD_XRAY_ANGIO
    SYNTAX_LIST_1            = JPEG_Syntax_List

    SERVICE_2                = STANDARD_GRAYSCALE_SOFTCOPY_PS
    SYNTAX_LIST_2            = ELE_ILE_EBE_Syntax_List

    SERVICE_3                = XRAY_RADIATION_DOSE_SR
    SYNTAX_LIST_3            = ELE_ILE_EBE_Syntax_List

    SERVICE_4                = STANDARD_SEC_CAPTURE
    SYNTAX_LIST_4            = JPEG_Syntax_List
  

[ELE_Syntax_List]
    SYNTAXES_SUPPORTED      = 1
    SYNTAX_2                = EXPLICIT_LITTLE_ENDIAN

[ILE_Syntax_List]
    SYNTAXES_SUPPORTED      = 1
    SYNTAX_2                = IMPLICIT_LITTLE_ENDIAN
	
[EBE_Syntax_List]
    SYNTAXES_SUPPORTED      = 1
    SYNTAX_1                = EXPLICIT_BIG_ENDIAN
	
[JPEG_Syntax_List]
    SYNTAXES_SUPPORTED      = 1
    SYNTAX_1                = JPEG_LOSSLESS_HIER_14
	

[Storage_Commit_SCU_Service_List]
    SERVICES_SUPPORTED      = 1       # Number of Services in list    
    SERVICE_1               = STORAGE_COMMITMENT_PUSH
    ROLE_1                  = SCU

[Storage_Commit_SCP_Service_List]
    SERVICES_SUPPORTED      = 1
    SERVICE_1               = STORAGE_COMMITMENT_PUSH
    ROLE_1                  = SCP

[Worklist_Service_List]
    SERVICES_SUPPORTED      = 1
    SERVICE_1               = MODALITY_WORKLIST_FIND
    SYNTAX_LIST_1           = ELE_ILE_EBE_Syntax_List

[ELE_ILE_EBE_Syntax_List]
    SYNTAXES_SUPPORTED      = 3
    SYNTAX_1                = EXPLICIT_LITTLE_ENDIAN
    SYNTAX_2                = IMPLICIT_LITTLE_ENDIAN
    SYNTAX_3                = EXPLICIT_BIG_ENDIAN

[Simulator_Worklist_Service_List]
    SERVICES_SUPPORTED = 2                        # Number of Services in list

    SERVICE_1          = MODALITY_WORKLIST_FIND
    SYNTAX_LIST_1      = ELE_ILE_EBE_Syntax_List

    SERVICE_2          = PERFORMED_PROCEDURE_STEP 
    SYNTAX_LIST_2      = ELE_ILE_EBE_Syntax_List

	
[MPPS_Service_List]
    SERVICES_SUPPORTED      = 1     
    SERVICE_1               = PERFORMED_PROCEDURE_STEP   
    SYNTAX_LIST_1           = ELE_ILE_EBE_Syntax_List   
	
[Verification_Service_List]
    SERVICES_SUPPORTED      = 1
    SERVICE_1               = STANDARD_ECHO	


[Full_Syntax_List]
    SYNTAXES_SUPPORTED      = 4
    SYNTAX_1                = EXPLICIT_LITTLE_ENDIAN
    SYNTAX_2                = IMPLICIT_LITTLE_ENDIAN
    SYNTAX_3                = EXPLICIT_BIG_ENDIAN
    SYNTAX_4                = JPEG_LOSSLESS_HIER_14