
#include "app.h"

// namespace ImGui	{

enum MyNodeTypes {
    MNT_COLOR_NODE = 0,
    MNT_COMBINE_NODE,
    MNT_COMMENT_NODE,
    MNT_COMPLEX_NODE,
#   ifdef IMGUI_USE_AUTO_BINDING
    MNT_TEXTURE_NODE,
#   endif
    MNT_OUTPUT_NODE,    // One problem here when adding new values is backward compatibility with old saved files: they rely on the previously used int values (it should be OK only if we append new values at the end).
    MNT_COUNT
};
// used in the "add Node" menu (and optionally as node title names)
static const char* MyNodeTypeNames[MNT_COUNT] = {"Color","Combine","Comment","Complex"
#						ifdef IMGUI_USE_AUTO_BINDING
						 ,"Texture"
#						endif
                        ,"Output"
						};
class Node : public ImGui::Node {
    ;
};
class ColorNode : public Node {
    protected:
    typedef Node Base;  //Base Class
    typedef ColorNode ThisClass;
    ColorNode() : Base() {}
    static const int TYPE = MNT_COLOR_NODE;

    ImVec4 Color;       // field

    // Support static method for enumIndex (the signature is the same used by ImGui::Combo(...))
    static bool GetTextFromEnumIndex(void* ,int value,const char** pTxt) {
        if (!pTxt) return false;
        static const char* values[] = {"APPLE","LEMON","ORANGE"};
        static int numValues = (int)(sizeof(values)/sizeof(values[0]));
        if (value>=0 && value<numValues) *pTxt = values[value];
        else *pTxt = "UNKNOWN";
        return true;
    }

    virtual const char* getTooltip() const {return "ColorNode tooltip.";}
    virtual const char* getInfo() const {return "ColorNode info.\n\nThis is supposed to display some info about this node.";}
    /*virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
        // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
        defaultTitleTextColorOut = IM_COL32(220,220,220,255);defaultTitleBgColorOut = IM_COL32(0,75,0,255);defaultTitleBgColorGradientOut = -1.f;
    }*/

    public:

    // create:
    static ThisClass* Create(const ImVec2& pos) {
        // 1) allocation
        // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
	// MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
	ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW (node) ThisClass();

        // 2) main init
        node->init("ColorNode",pos,"","r;g;b;a",TYPE);

        // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        node->fields.addFieldColor(&node->Color.x,true,"Color","color with alpha");

        // 4) set (or load) field values
        node->Color = ImColor(255,255,0,255);

        return node;
    }


    // casts:
    inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
};
class CombineNode : public Node {
    protected:
    typedef Node Base;  //Base Class
    typedef CombineNode ThisClass;
    CombineNode() : Base() {}
    static const int TYPE = MNT_COMBINE_NODE;

    float fraction;

    virtual const char* getTooltip() const {return "CombineNode tooltip.";}
    virtual const char* getInfo() const {return "CombineNode info.\n\nThis is supposed to display some info about this node.";}
    /*virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
        // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
        defaultTitleTextColorOut = IM_COL32(220,220,220,255);defaultTitleBgColorOut = IM_COL32(0,75,0,255);defaultTitleBgColorGradientOut = -1.f;
    }*/

    public:

    // create:
    static ThisClass* Create(const ImVec2& pos) {
        // 1) allocation
        // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
	// MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
	ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW(node) ThisClass();

        // 2) main init
        node->init("CombineNode",pos,"in1;in2","out",TYPE);

        // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
	node->fields.addField(&node->fraction,1,"Fraction","Fraction of in1 that is mixed with in2",2,0,1);

        // 4) set (or load) field values
        node->fraction = 0.5f;

        return node;
    }

    // casts:
    inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}


};
class CommentNode : public Node {
    protected:
    typedef Node Base;  //Base Class
    typedef CommentNode ThisClass;
    CommentNode() : Base() {}
    static const int TYPE = MNT_COMMENT_NODE;
    static const int TextBufferSize = 128;

    char comment[TextBufferSize];			    // field 1
    char comment2[TextBufferSize];			    // field 2
    char comment3[TextBufferSize];			    // field 3
    char comment4[TextBufferSize];			    // field 4
    bool flag;                                  // field 5

    virtual const char* getTooltip() const {return "CommentNode tooltip.";}
    virtual const char* getInfo() const {return "CommentNode info.\n\nThis is supposed to display some info about this node.";}
    /*virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
        // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
        defaultTitleTextColorOut = IM_COL32(220,220,220,255);defaultTitleBgColorOut = IM_COL32(0,75,0,255);defaultTitleBgColorGradientOut = -1.f;
    }*/

    public:

    // create:
    static ThisClass* Create(const ImVec2& pos) {
	// 1) allocation
	// MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
	// MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
	ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW(node) ThisClass();

	// 2) main init
	node->init("CommentNode",pos,"","",TYPE);
	node->baseWidthOverride = 200.f;    // (optional) default base node width is 120.f;


	// 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
	node->fields.addFieldTextEdit(		&node->comment[0],TextBufferSize,"Single Line","A single line editable field",ImGuiInputTextFlags_EnterReturnsTrue);
	node->fields.addFieldTextEditMultiline(&node->comment2[0],TextBufferSize,"Multi Line","A multi line editable field",ImGuiInputTextFlags_AllowTabInput,50);
	node->fields.addFieldTextEditMultiline(&node->comment3[0],TextBufferSize,"Multi Line 2","A multi line read-only field",ImGuiInputTextFlags_ReadOnly,50);
	node->fields.addFieldTextWrapped(      &node->comment4[0],TextBufferSize,"Text Wrapped ReadOnly","A text wrapped field");
	node->fields.addField(&node->flag,"Flag","A boolean field");

	// 4) set (or load) field values
	strcpy(node->comment,"Initial Text Line.");
	strcpy(node->comment2,"Initial Text Multiline.");
	static const char* tiger = "Tiger, tiger, burning bright\nIn the forests of the night,\nWhat immortal hand or eye\nCould frame thy fearful symmetry?";
	strncpy(node->comment3,tiger,TextBufferSize);
	static const char* txtWrapped = "I hope this text gets wrapped gracefully. But I'm not sure about it.";
	strncpy(node->comment4,txtWrapped,TextBufferSize);
	node->flag = true;

	return node;
    }

    // helper casts:
    inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
};
class ComplexNode : public Node {
    protected:
    typedef Node Base;  //Base Class
    typedef ComplexNode ThisClass;
    ComplexNode() : Base() {}
    static const int TYPE = MNT_COMPLEX_NODE;

    float Value[3];     // field 1
    ImVec4 Color;       // field 2
    int enumIndex;      // field 3

    // Support static method for enumIndex (the signature is the same used by ImGui::Combo(...))
    static bool GetTextFromEnumIndex(void* ,int value,const char** pTxt) {
        if (!pTxt) return false;
        static const char* values[] = {"APPLE","LEMON","ORANGE"};
        static int numValues = (int)(sizeof(values)/sizeof(values[0]));
        if (value>=0 && value<numValues) *pTxt = values[value];
        else *pTxt = "UNKNOWN";
        return true;
    }

    virtual const char* getTooltip() const {return "ComplexNode tooltip.";}
    virtual const char* getInfo() const {return "ComplexNode info.\n\nThis is supposed to display some info about this node.";}
    virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
        // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
        // defaultTitleTextColorOut = IM_COL32(220,220,220,255);defaultTitleBgColorOut = IM_COL32(125,35,0,255);defaultTitleBgColorGradientOut = -1.f;
    }

    public:

    // create:
    static ThisClass* Create(const ImVec2& pos) {
        // 1) allocation
        // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
        // MANDATORY even with blank ctrs.  Reason: ImVector does not call ctrs/dctrs on items.
        ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW(node) ThisClass();

        // 2) main init
        node->init("ComplexNode",pos,"in1;in2;in3","out1;out2",TYPE);

        // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        node->fields.addField(&node->Value[0],3,"Angles","Three floats that are stored in radiant units internally",2,0,360,NULL,true);
        node->fields.addFieldColor(&node->Color.x,true,"Color","color with alpha");
        // addFieldEnum(...) now has all the 3 overloads ImGui::Combo(...) has.
        // addFieldEnum(...) [1] (item_count + external callback)
        node->fields.addFieldEnum(&node->enumIndex,3,&GetTextFromEnumIndex,"Fruit","Choose your favourite");
        // addFieldEnum(...) [2] (items_count + item_names)
        //static const char* FruitNames[3] = {"APPLE","LEMON","ORANGE"};
        //node->fields.addFieldEnum(&node->enumIndex,3,FruitNames,"Fruit","Choose your favourite");
        // addFieldEnum(...) [3] (zero_separated_item_names)
        //node->fields.addFieldEnum(&node->enumIndex,"APPLE\0LEMON\0ORANGE\0\0","Fruit","Choose your favourite");


        // 4) set (or load) field values
        node->Value[0] = 0;node->Value[1] = 3.14f; node->Value[2] = 4.68f;
        node->Color = ImColor(126,200,124,230);
        node->enumIndex = 1;

        return node;
    }

    // helper casts:
    inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
};
#ifdef IMGUI_USE_AUTO_BINDING
class TextureNode : public Node {
    protected:
    typedef Node Base;  //Base Class
    typedef TextureNode ThisClass;
    TextureNode() : Base() {}
    virtual ~TextureNode() {if (textureID) {ImImpl_FreeTexture(textureID);}}
    static const int TYPE = MNT_TEXTURE_NODE;
    static const int TextBufferSize =
#   ifndef NO_IMGUIFILESYSTEM
    ImGuiFs::MAX_PATH_BYTES;
#   else
    2049;
#   endif

    ImTextureID textureID;
    char imagePath[TextBufferSize];				// field 1 (= the only one that is copied/serialized/handled by the Node)
    char lastValidImagePath[TextBufferSize];    // The path for which "textureID" was created
    bool startBrowseDialogNextFrame;
#   ifndef NO_IMGUIFILESYSTEM
    ImGuiFs::Dialog dlg;
#   endif //NO_IMGUIFILESYSTEM

    virtual const char* getTooltip() const {return "TextureNode tooltip.";}
    virtual const char* getInfo() const {return "TextureNode info.\n\nThis is supposed to display some info about this node.";}
    virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
        // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
        defaultTitleTextColorOut = IM_COL32(220,220,220,255);defaultTitleBgColorOut = IM_COL32(105,105,0,255);defaultTitleBgColorGradientOut = -1.f;
    }

    public:

    // create (main method of this class):
    static ThisClass* Create(const ImVec2& pos) {
	// 1) allocation
	// MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
	// MANDATORY even with blank ctrs.  Reason: ImVector does not call ctrs/dctrs on items.
	ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));
	IM_PLACEMENT_NEW(node) ThisClass();

	// 2) main init
	node->init("TextureNode",pos,"","r;g;b;a",TYPE);
	node->baseWidthOverride = 150.f;    // (optional) default base node width is 120.f;

	// 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
	FieldInfo* f=NULL;
#	ifndef NO_IMGUIFILESYSTEM
	f=&node->fields.addFieldTextEditAndBrowseButton(&node->imagePath[0],TextBufferSize,"Image Path:","A valid image path: press RETURN to validate or browse manually.",ImGuiInputTextFlags_EnterReturnsTrue,(void*) node);
#	else	//NO_IMGUIFILESYSTEM
	f=&node->fields.addFieldTextEdit(&node->imagePath[0],TextBufferSize,"Image Path:","A valid image path: press RETURN to validate or browse manually.",ImGuiInputTextFlags_EnterReturnsTrue,(void*) node);
#	endif //NO_IMGUIFILESYSTEM
    f->editedFieldDelegate = &ThisClass::StaticEditFieldCallback;   // we set an "edited callback" to this node field. It is fired soon (as opposed to other "outer" edited callbacks), but we have chosen: ImGuiInputTextFlags_EnterReturnsTrue.
	node->startBrowseDialogNextFrame = false;

	// 4) set (or load) field values
	node->textureID = 0;
	node->imagePath[0] = node->lastValidImagePath[0] = '\0';

	return node;
    }

    // helper casts:
    inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}

    protected:
    bool render(float nodeWidth)   {
	const bool changed = Base::render(nodeWidth);
#	ifndef NO_IMGUIFILESYSTEM
	const char* filePath = dlg.chooseFileDialog(startBrowseDialogNextFrame,dlg.getLastDirectory(),".jpg;.jpeg;.png;.gif;.tga;.bmp");
	if (strlen(filePath)>0) {
	    //fprintf(stderr,"Browsed..: %s\n",filePath);
	    strcpy(imagePath,filePath);
	    processPath(imagePath);
	}
#	endif //NO_IMGUIFILESYSTEM
	startBrowseDialogNextFrame = false;
	//----------------------------------------------------
	// draw textureID:
	ImGui::Image(reinterpret_cast<void*>(textureID),ImVec2(nodeWidth,nodeWidth));
	//----------------------------------------------------
	return changed;
    }

    void processPath(const char* filePath)  {
        if (!filePath || strcmp(filePath,lastValidImagePath)==0) return;
        if (!ValidateImagePath(filePath)) return;
        if (textureID) {ImImpl_FreeTexture(textureID);}
        textureID = ImImpl_LoadTexture(filePath);
        if (textureID) strcpy(lastValidImagePath,filePath);
    }

    // When the node is loaded from file or copied from another node, only the text field (="imagePath") is copied, so we must recreate "textureID":
    void onCopied() {
        processPath(imagePath);
    }
    void onLoaded() {
        processPath(imagePath);
    }
    //bool canBeCopied() const {return false;}	// Just for testing... TO REMOVE!

    void onEditField(FieldInfo& /*f*/,int widgetIndex) {
        //fprintf(stderr,"TextureNode::onEditField(\"%s\",%i);\n",f.label,widgetIndex);
        if (widgetIndex==1)         startBrowseDialogNextFrame = true;  // browsing button pressed
        else if (widgetIndex==0)    processPath(imagePath);             // text edited (= "return" pressed in out case)
    }
    static void StaticEditFieldCallback(FieldInfo& f,int widgetIndex) {
        reinterpret_cast<ThisClass*>(f.userData)->onEditField(f,widgetIndex);
    }

    static bool ValidateImagePath(const char* path) {
        if (!path || strlen(path)==0) return false;

        // TODO: check extension

        FILE* f = ImFileOpen(path,"rb");
        if (f) {fclose(f);f=NULL;return true;}

        return false;
    }

};
#endif //IMGUI_USE_AUTO_BINDING
class OutputNode : public Node {
    protected:
    typedef Node Base;  //Base Class
    typedef OutputNode ThisClass;
    OutputNode() : Base() {}
    static const int TYPE = MNT_OUTPUT_NODE;

    // No field values in this class
    // float volume;

    virtual const char* getTooltip() const {
        return "OutputNode";
    }
    virtual const char* getInfo() const {
        return "OutputNode: Audio coming into this node goes to your speakers.";
    }
    virtual void getDefaultTitleBarColors(ImU32& defaultTitleTextColorOut,ImU32& defaultTitleBgColorOut,float& defaultTitleBgColorGradientOut) const {
        // [Optional Override] customize Node Title Colors [default values: 0,0,-1.f => do not override == use default values from the Style()]
        defaultTitleTextColorOut = IM_COL32_BLACK;
        defaultTitleBgColorOut = ImGui::GetColorU32(ImGuiCol_PlotHistogram);
        // defaultTitleBgColorGradientOut = 1.0f;
    }
    virtual bool canBeCopied() const {return false;}

    public:

    // create:
    static ThisClass* Create(const ImVec2& pos) {
        // 1) allocation
        // MANDATORY (NodeGraphEditor::~NodeGraphEditor() will delete these with ImGui::MemFree(...))
    // MANDATORY even with blank ctrs. Reason: ImVector does not call ctrs/dctrs on items.
    ThisClass* node = (ThisClass*) ImGui::MemAlloc(sizeof(ThisClass));IM_PLACEMENT_NEW (node) ThisClass();

        // 2) main init
        node->init("OutputNode",pos,"in1;in2;in3;in4","",TYPE);

        // 3) init fields ( this uses the node->fields variable; otherwise we should have overridden other virtual methods (to render and serialize) )
        // node->fields.addField(
        //     &node->volume,
        //     1, // numArrayElements
        //     "Volume",
        //     "Global output volume",
        //     2, // precision
        //     0, // lower
        //     1  // upper
        // );

        // 4) set (or load) field values
        // node->volume = appState.audio.getGlobalVolume();

        return node;
    }

    // casts:
    inline static ThisClass* Cast(Node* n) {return Node::Cast<ThisClass>(n,TYPE);}
    inline static const ThisClass* Cast(const Node* n) {return Node::Cast<ThisClass>(n,TYPE);}

    protected:
    bool render(float /*nodeWidth*/)   {
        // ImGui::Text("There can be a single\ninstance of this class.\nTry and see if it's true!");

        if (ImGui::SliderFloat("Volume", &appState.volume, 0.0f, 1.0f)) {
            appState.audio.setVolume(appState.audio_handle, appState.volume);
        }

        ImGui::PlotLines(
            "##Live Waveform",
            appState.audio.getWave(),
            256,  // values_count
            0,    // values_offset
            nullptr, // overlay_text
            -1.0f, // scale_min
            1.0f, // scale_max
            ImVec2(200,100) // graph_size
        );


        return false;
    }
};

static ImGui::Node* MyNodeFactory(int nt,const ImVec2& pos,const ImGui::NodeGraphEditor& /*nge*/) {
    switch (nt) {
    case MNT_COLOR_NODE: return ColorNode::Create(pos);
    case MNT_COMBINE_NODE: return CombineNode::Create(pos);
    case MNT_COMMENT_NODE: return CommentNode::Create(pos);
    case MNT_COMPLEX_NODE: return ComplexNode::Create(pos);
    case MNT_OUTPUT_NODE: return OutputNode::Create(pos);
#   ifdef IMGUI_USE_AUTO_BINDING
    case MNT_TEXTURE_NODE: return TextureNode::Create(pos);
#   endif //IMGUI_USE_AUTO_BINDING
    default:
    IM_ASSERT(true);    // Missing node type creation
    return NULL;
    }
    return NULL;
}
void MyNodeGraphEditor(ImGui::NodeGraphEditor & nge)  {
    if (nge.isInited())	{
        // This adds entries to the "add node" context menu
        nge.registerNodeTypes(MyNodeTypeNames,MNT_COUNT,MyNodeFactory,NULL,-1); // last 2 args can be used to add only a subset of nodes (or to sort their order inside the context menu)
        // The line above can be replaced by the following two lines, if we want to use only an active subset of the available node types:
        //const int optionalNodeTypesToUse[] = {MNT_COMPLEX_NODE,MNT_COMMENT_NODE,MNT_OUTPUT_NODE};
        //nge.registerNodeTypes(MyNodeTypeNames,MNT_COUNT,MyNodeFactory,optionalNodeTypesToUse,sizeof(optionalNodeTypesToUse)/sizeof(optionalNodeTypesToUse[0]));
        nge.registerNodeTypeMaxAllowedInstances(MNT_OUTPUT_NODE,1); // Here we set the max number of allowed instances of the output node (1)

        // Optional: starting nodes and links (TODO: load from file instead):-----------
        ImGui::Node* colorNode = nge.addNode(MNT_COLOR_NODE,ImVec2(40,50));
        ImGui::Node* complexNode =  nge.addNode(MNT_COMPLEX_NODE,ImVec2(40,150));
        ImGui::Node* combineNode =  nge.addNode(MNT_COMBINE_NODE,ImVec2(325,80)); // optionally use e.g.: ImGui::CombineNode::Cast(combineNode)->fraction = 0.8f;
        ImGui::Node* outputNode =  nge.addNode(MNT_OUTPUT_NODE,ImVec2(620,140));
        // Return values can be NULL (if node types are not registered or their instance limit has been already reached).
        //nge.overrideNodeName(combineNode,"CombineNodeCustomName");  // Test only (to remove)
        //nge.overrideNodeInputSlots(combineNode,"in1;in2;in3;in4");  // Test only (to remove)
        //ImU32 bg = IM_COL32(0,128,0,255);nge.overrideNodeTitleBarColors(combineNode,NULL,&bg,NULL);  // Test only (to remove)
        //nge.overrideNodeTitleBarColors(complexNode,NULL,&bg,NULL);  // Test only (to remove)
        // addLink(...) should be robust enough to handle NULL nodes, so we don't check it.
        nge.addLink(colorNode, 0, combineNode, 0);
        nge.addLink(complexNode, 1, combineNode, 1);
        nge.addLink(complexNode, 0, outputNode, 1);
        nge.addLink(combineNode, 0, outputNode, 0);
        //-------------------------------------------------------------------------------
        //nge.load("nodeGraphEditor.nge");  // Please note than if the saved graph has nodes out of our active subset, they will be displayed as usual (it's not clear what should be done in this case: hope that's good enough, it's a user's mistake).
        //-------------------------------------------------------------------------------
        nge.show_style_editor = true;
        nge.show_load_save_buttons = true;
        nge.show_node_copy_paste_buttons = false;
        // ImGui::NodeGraphEditor::CloseCopyPasteChars[0] = strdup("X");
        // ImGui::NodeGraphEditor::CloseCopyPasteChars[1] = strdup("C");
        // ImGui::NodeGraphEditor::CloseCopyPasteChars[2] = strdup("P");
        // optional load the style (for all the editors: better call it in InitGL()):
        // ImGui::NodeGraphEditor::Style::Load(ImGui::NodeGraphEditor::GetStyle(),"nodeGraphEditor.nge.style");
        //--------------------------------------------------------------------------------
    }
    nge.render();
}

// }	//nmespace ImGui



