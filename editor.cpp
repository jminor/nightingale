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

#include <map>

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



NodeEditor::PinId DrawNode(ma_node* node)
{
    ma_node_base* node_base = (ma_node_base*)node;

    NodeEditor::BeginNode((NodeEditor::NodeId)node);
    ImGui::PushID(node);

    ImGui::PushItemWidth(80);

    bool running = ma_node_get_state(node) == ma_node_state_started;
    if (ImGui::Checkbox("Active", &running)) {
        ma_node_set_state(node, running ? ma_node_state_started : ma_node_state_stopped);
    }

    float time = ma_node_get_time(node);
    ImGui::DragFloat("Time", &time);

    std::map<ma_node_input_bus*, ma_node*> more_nodes;

    int input_bus_count = ma_node_get_input_bus_count(node);
    for (int bus_index = 0; bus_index < input_bus_count; bus_index++)
    {
        int input_channels = ma_node_get_input_channels(node, bus_index);
        ma_node_input_bus* input_bus = &node_base->pInputBuses[bus_index];

        auto input_attr_id = (NodeEditor::PinId) input_bus;
        NodeEditor::BeginPin(input_attr_id, NodeEditor::PinKind::Input);
        ImGui::Text(">");
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
            ma_node* another = output_bus->pNode;
            if (another != nullptr) {
                more_nodes[input_bus] = another;
            }
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

    for (auto& pair : more_nodes) {
        const auto& input_attr_id = (NodeEditor::PinId) pair.first;
        auto& another_node = pair.second;

        auto other_pin_id = DrawNode(another_node);

        static int link_id = 0;
        NodeEditor::Link(link_id++, other_pin_id, input_attr_id);
    }

    return output_attr_id;
}


static NodeEditor::EditorContext* __NodeEditor = nullptr;


void GraphEditor()
{
    ImGui::Text("Graph Editor");

    if (__NodeEditor == NULL) {
        NodeEditor::Config config;
        config.SettingsFile = "Simple.json";
        __NodeEditor = NodeEditor::CreateEditor(&config);
    }

    NodeEditor::SetCurrentEditor(__NodeEditor);

    NodeEditor::Begin("My Editor", ImVec2(0.0, 0.0f));

    ma_node_graph* graph = node_graph();

    ma_node* node = ma_node_graph_get_endpoint(graph);
    DrawNode(node);

    NodeEditor::End();

    NodeEditor::SetCurrentEditor(NULL);
}
