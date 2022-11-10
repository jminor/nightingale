//
//  editor.cpp
//  nightingale
//
//  Created by Joshua Minor on 11/6/22.
//

#include "editor.h"

#include "imgui.h"
#include "imgui_node_editor.h"
namespace NodeEditor = ax::NodeEditor;

#include "audio.h"

#include <unordered_map>

std::unordered_map<ma_node_input_bus*, std::pair<ma_node*, int>> input_bus_map;

#define c89atomic_load_ptr(x) (x)
#define MA_ASSERT(x)


static ma_uint32 ma_node_input_bus_get_channels(const ma_node_input_bus* pInputBus)
{
    return pInputBus->channels;
}

static float* ma_node_get_cached_input_ptr(ma_node* pNode, ma_uint32 inputBusIndex)
{
    ma_node_base* pNodeBase = (ma_node_base*)pNode;
    ma_uint32 iInputBus;
    float* pBasePtr;

    MA_ASSERT(pNodeBase != NULL);

    /* Input data is stored at the front of the buffer. */
    pBasePtr = pNodeBase->pCachedData;
    for (iInputBus = 0; iInputBus < inputBusIndex; iInputBus += 1) {
        pBasePtr += pNodeBase->cachedDataCapInFramesPerBus * ma_node_input_bus_get_channels(&pNodeBase->pInputBuses[iInputBus]);
    }

    return pBasePtr;
}

static ma_uint32 ma_node_output_bus_get_channels(const ma_node_output_bus* pOutputBus)
{
    return pOutputBus->channels;
}

static float* ma_node_get_cached_output_ptr(ma_node* pNode, ma_uint32 outputBusIndex)
{
    ma_node_base* pNodeBase = (ma_node_base*)pNode;
    ma_uint32 iInputBus;
    ma_uint32 iOutputBus;
    float* pBasePtr;

    MA_ASSERT(pNodeBase != NULL);

    /* Cached output data starts after the input data. */
    pBasePtr = pNodeBase->pCachedData;
    for (iInputBus = 0; iInputBus < ma_node_get_input_bus_count(pNodeBase); iInputBus += 1) {
        pBasePtr += pNodeBase->cachedDataCapInFramesPerBus * ma_node_input_bus_get_channels(&pNodeBase->pInputBuses[iInputBus]);
    }

    for (iOutputBus = 0; iOutputBus < outputBusIndex; iOutputBus += 1) {
        pBasePtr += pNodeBase->cachedDataCapInFramesPerBus * ma_node_output_bus_get_channels(&pNodeBase->pOutputBuses[iOutputBus]);
    }

    return pBasePtr;
}

static int link_id = 0;

//static ma_node_vtable* g_ma_lpf_node_vtable;
extern ma_node_vtable g_ma_lpf_node_vtable;


NodeEditor::PinId DrawNode(ma_node* node)
{
    ma_node_base* node_base = (ma_node_base*)node;

    NodeEditor::BeginNode((NodeEditor::NodeId)node);
    ImGui::PushID(node);

    ImGui::PushItemWidth(80);

    const char* title = "?";
    
    if (node_base->vtable == &g_ma_lpf_node_vtable ) {
        title = "Low Pass Filter";
    }
    ImGui::Text("%s", title);

    bool running = ma_node_get_state(node) == ma_node_state_started;
    if (ImGui::Checkbox("Active", &running)) {
        ma_node_set_state(node, running ? ma_node_state_started : ma_node_state_stopped);
    }

    float time = ma_node_get_time(node);
    ImGui::Text("Time: %d", (int)time);

    std::unordered_map<ma_node_input_bus*, ma_node_output_bus*> links;

    int input_bus_count = ma_node_get_input_bus_count(node);
    for (int bus_index = 0; bus_index < input_bus_count; bus_index++)
    {
        int input_channels = ma_node_get_input_channels(node, bus_index);
        ma_node_input_bus* input_bus = &node_base->pInputBuses[bus_index];
        input_bus_map[input_bus] = std::pair<ma_node*, int>(node, bus_index);

        auto input_attr_id = (NodeEditor::PinId) input_bus;
        NodeEditor::BeginPin(input_attr_id, NodeEditor::PinKind::Input);
        ImGui::Text(">");
//        NodeEditor::PinPivotRect(inputsRect.GetCenter(), inputsRect.GetCenter());
//        NodeEditor::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
        NodeEditor::EndPin();

        ImGui::SameLine();
        ImGui::Text("IN %d: %d channels", bus_index, input_channels);


        float *cache = ma_node_get_cached_input_ptr(node, bus_index);
        int count = node_base->cachedDataCapInFramesPerBus;

        if (cache != NULL && count > 0) {
            ImGui::PlotLines(
                             "##PCM",
                             cache,
                             count,
                             0,    // values_offset
                             nullptr, // overlay_text
                             -1.0f, // scale_min
                             1.0f, // scale_max
                             ImVec2(80,40) // graph_size
                             );
        }

        if (input_bus == nullptr) continue;

        ma_node_output_bus* output_bus;
        for (output_bus = (ma_node_output_bus*)c89atomic_load_ptr(input_bus->head.pNext);
             output_bus != NULL;
             output_bus = (ma_node_output_bus*)c89atomic_load_ptr(output_bus->pNext))
        {
//            links[input_bus] = output_bus;
            const auto& input_attr_id = (NodeEditor::PinId) input_bus;
            const auto& output_pin_id = (NodeEditor::PinId) output_bus;

            auto link_id = (NodeEditor::LinkId)((char*)output_pin_id.AsPointer() + 1);
            NodeEditor::Link(link_id, output_pin_id, input_attr_id);
        }

    }

    float volume = ma_node_get_output_bus_volume(node, 0);
    if (ImGui::DragFloat("OUT", &volume, 0.01, 0.0, 1.0)) {
        ma_node_set_output_bus_volume(node, 0, volume);
    }

    ImGui::SameLine();
    auto output_attr_id = (NodeEditor::PinId) &node_base->pOutputBuses[0];
    NodeEditor::BeginPin(output_attr_id, NodeEditor::PinKind::Output);
    ImGui::Text(">");
    NodeEditor::EndPin();

    float *cache = ma_node_get_cached_output_ptr(node, 0);
    int count = node_base->cachedFrameCountOut;
    if (cache != NULL && count > 0) {
        ImGui::PlotLines(
                         "##OUT",
                         cache,
                         count,
                         0,    // values_offset
                         nullptr, // overlay_text
                         -1.0f, // scale_min
                         1.0f, // scale_max
                         ImVec2(80,40) // graph_size
                         );
    }

//    ImGui::Dummy(ImVec2(80.0f, 45.0f));
    ImGui::PopID();
    NodeEditor::EndNode();

    ImGui::PopItemWidth();

    for (auto& pair : links) {
        const auto& input_attr_id = (NodeEditor::PinId) pair.first;
        const auto& output_pin_id = (NodeEditor::PinId) pair.second;

        auto link_id = (NodeEditor::LinkId) output_pin_id.AsPointer();
        NodeEditor::Link(link_id, output_pin_id, input_attr_id);
        // This *looks* cool, but causes crashes if you delete a node?!
//        NodeEditor::Flow(link_id);
//        link_id++;
    }

    return output_attr_id;
}


static NodeEditor::EditorContext* __NodeEditor = nullptr;


void GraphEditor()
{
    ImGui::Text("Graph Editor");

    bool first_frame = false;
    if (__NodeEditor == NULL) {
        NodeEditor::Config config;
        config.SettingsFile = "Simple.json";
        __NodeEditor = NodeEditor::CreateEditor(&config);
        first_frame = true;
    }

    NodeEditor::SetCurrentEditor(__NodeEditor);

    NodeEditor::Begin("My Editor", ImVec2(0.0, 0.0f));

    link_id = 0;

    ma_node_graph* graph = node_graph();

    input_bus_map.clear();

    for (auto& node : all_nodes()) {
        DrawNode(node);
    }

    if (NodeEditor::BeginCreate())
    {
        NodeEditor::PinId inputPinId, outputPinId;
        if (NodeEditor::QueryNewLink(&inputPinId, &outputPinId))
        {
            // QueryNewLink returns true if editor want to create new link between pins.
            //
            // Link can be created only for two valid pins, it is up to you to
            // validate if connection make sense. Editor is happy to make any.
            //
            // Link always goes from input to output. User may choose to drag
            // link from output pin or input pin. This determine which pin ids
            // are valid and which are not:
            //   * input valid, output invalid - user started to drag new ling from input pin
            //   * input invalid, output valid - user started to drag new ling from output pin
            //   * input valid, output valid   - user dragged link over other pin, can be validated

            if (inputPinId && outputPinId) // both are valid, let's accept link
            {
                // NodeEditor::AcceptNewItem() return true when user release mouse button.
                if (NodeEditor::AcceptNewItem())
                {
                    ma_node_input_bus* input_bus = (ma_node_input_bus*)inputPinId.AsPointer();
                    ma_node_output_bus* output_bus = (ma_node_output_bus*)outputPinId.AsPointer();

                    auto& pair = input_bus_map[input_bus];

                    if (pair.first == NULL) {
                        // wired backwards, swap
                        pair = input_bus_map[(ma_node_input_bus*)output_bus];
                        output_bus = (ma_node_output_bus*)input_bus;
                    }

                    auto input_node = pair.first;
                    auto input_bus_index = pair.second;

                    ma_node_attach_output_bus(output_bus->pNode,
                                              output_bus->outputBusIndex,
                                              input_node,
                                              input_bus_index);
                }

                // You may choose to reject connection between these nodes
                // by calling NodeEditor::RejectNewItem(). This will allow editor to give
                // visual feedback by changing link thickness and color.
            }
        }
    }
    NodeEditor::EndCreate(); // Wraps up object creation action handling.


    // Handle deletion action
    if (NodeEditor::BeginDelete())
    {
        // There may be many links marked for deletion, let's loop over them.
        NodeEditor::LinkId deletedLinkId;
        while (NodeEditor::QueryDeletedLink(&deletedLinkId))
        {
            // If you agree that link can be deleted, accept deletion.
            if (NodeEditor::AcceptDeletedItem())
            {
                ma_node_output_bus* output_bus = (ma_node_output_bus*)((char*)deletedLinkId.AsPointer() - 1);
                ma_node_detach_output_bus(output_bus->pNode, output_bus->outputBusIndex);
            }

            // You may reject link deletion by calling:
            // NodeEditor::RejectDeletedItem();
        }

        NodeEditor::NodeId deletedNodeId;
        while (NodeEditor::QueryDeletedNode(&deletedNodeId))
        {
            if (NodeEditor::AcceptDeletedItem())
            {
                ma_node_base* node = (ma_node_base*)deletedNodeId.AsPointer();
                ma_node_detach_all_output_buses(node);
            }
        }
    }
    NodeEditor::EndDelete(); // Wrap up deletion action


    auto openPopupPosition = ImGui::GetMousePos();
    static NodeEditor::NodeId contextNodeId      = 0;
    static NodeEditor::LinkId contextLinkId      = 0;
    static NodeEditor::PinId  contextPinId       = 0;
    NodeEditor::Suspend();
    if (NodeEditor::ShowNodeContextMenu(&contextNodeId))
        ImGui::OpenPopup("Node Context Menu");
    else if (NodeEditor::ShowPinContextMenu(&contextPinId))
        ImGui::OpenPopup("Pin Context Menu");
    else if (NodeEditor::ShowLinkContextMenu(&contextLinkId))
        ImGui::OpenPopup("Link Context Menu");
    else if (NodeEditor::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("Create New Node");
    }
    NodeEditor::Resume();

    NodeEditor::Suspend();
    if (ImGui::BeginPopup("Node Context Menu"))
    {
        auto node = (ma_node_base*)contextNodeId.AsPointer();

        ImGui::TextUnformatted("Node Context Menu");
        ImGui::Separator();
        if (node)
        {
            ImGui::Text("State: %s", node->state == ma_node_state_started ? "running" : "stopped");
//            ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : (node->Type == NodeType::Tree ? "Tree" : "Comment"));
            ImGui::Text("Inputs: %d", (int)node->inputBusCount);
            ImGui::Text("Outputs: %d", (int)node->outputBusCount);
        }
        else {
            ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete")) {
//            ma_node_detach_all_output_buses(node);
            // or this:
            NodeEditor::DeleteNode(contextNodeId);
        }
        ImGui::EndPopup();
    }

/*
 if (ImGui::BeginPopup("Pin Context Menu"))
    {
        if (ImGui::MenuItem("Disconnect")) {
            ...
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Link Context Menu"))
    {
        auto link = FindLink(contextLinkId);
        if (ImGui::MenuItem("Delete"))
            NodeEditor::DeleteLink(contextLinkId);
        ImGui::EndPopup();
    }
*/

    if (ImGui::BeginPopup("Create New Node"))
    {
        auto newNodePostion = openPopupPosition;
        //ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

//        auto drawList = ImGui::GetWindowDrawList();
//        drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

        ma_node* node = nullptr;
        ma_result result;

        if (ImGui::MenuItem("Low Pass Filter")) {
            ma_lpf_node_config lpfNodeConfig =
                ma_lpf_node_config_init(num_channels(),
                                        sample_rate(),
                                        sample_rate() / 80,
                                        8);

            node = new ma_lpf_node;
            result = ma_lpf_node_init(node_graph(), &lpfNodeConfig, NULL, (ma_lpf_node*)node);
            if (result != MA_SUCCESS) {
                printf("ERROR: Failed to initialize low pass filter node.");
            }
        }

        if (ImGui::MenuItem("Echo / Delay")) {
            ma_delay_node_config delayNodeConfig =
                ma_delay_node_config_init(num_channels(),
                                          sample_rate(),
                                          (ma_uint32)(sample_rate() * 0.2f),
                                          0.5f);

            node = new ma_delay_node;
            result = ma_delay_node_init(node_graph(), &delayNodeConfig, NULL, (ma_delay_node*)node);
            if (result != MA_SUCCESS) {
                printf("ERROR: Failed to initialize delay node.");
            }
        }

        if (ImGui::MenuItem("Splitter")) {
            ma_splitter_node_config splitterNodeConfig = ma_splitter_node_config_init(num_channels());

            node = new ma_splitter_node;
            result = ma_splitter_node_init(node_graph(), &splitterNodeConfig, NULL, (ma_splitter_node*)node);
            if (result != MA_SUCCESS) {
                printf("ERROR: Failed to initialize splitter node.");
                return -1;
            }
        }

        if (node)
        {
            NodeEditor::SetNodePosition((NodeEditor::NodeId)node, newNodePostion);
            all_nodes().insert(node);
        }

        ImGui::EndPopup();
    }
    NodeEditor::Resume();

    NodeEditor::End();

    if (first_frame) {
        NodeEditor::NavigateToContent(0.0f);
    }

    NodeEditor::SetCurrentEditor(NULL);
}
