import sys
import json
import pandas as pd
import networkx as nx

def read_files(input_files: dict) -> tuple:
    general_data_path = input_files['generalData']
    graph_data_path = input_files['graphData']
    cliques_data_path = input_files['cliquesData']

    general_data = pd.read_csv(general_data_path, sep=';')
    graph_data = nx.read_gpickle(graph_data_path)
    cliques_data = json.load(open(cliques_data_path))

    return (general_data, graph_data, cliques_data)

# Generates the dictionary which contains the generator functions for each metric
def get_metrics_dict():
    def density(graph: nx.Graph) -> float:
        if graph is None: return 0
        return nx.density(graph)

    def competition(graph: nx.Graph) -> float:
        if graph is None: return 1
        connected_components = nx.number_connected_components(graph)
        vertices = graph.number_of_nodes()
        if vertices != 0:
            return connected_components / vertices
        else:
            return float('NaN')
    
    def number_of_nodes(graph: nx.Graph) -> int:
        if graph is None: return 0
        return graph.number_of_nodes()

    def number_of_edges(graph: nx.Graph) -> int:
        if graph is None: return 0
        return graph.number_of_edges()

    def number_of_cliques(bidding_cliques: list) -> int:
        if bidding_cliques is not None:
            return len(bidding_cliques)
        else:
            return 0

    def size_of_largest_clique(bidding_cliques: list) -> int:
        if bidding_cliques is not None:
            return max(len(clique) for clique in bidding_cliques)
        else:
            return 0

    metric_generators = {
        "density": density,
        "competition": competition,
        "nbOfNodes": number_of_nodes,
        "nbOfEdges": number_of_edges,
        "nbOfCliques": number_of_cliques,
        "sizeOfLargestClique": size_of_largest_clique,
    }
    return metric_generators       

def output_metrics(metrics: dict, output_file_path: str, metric_list: list):
    metrics_df = pd.DataFrame.from_dict(metrics, orient='index', 
                                        columns=["id_item", *metric_list, "vlr_item"])
    metrics_df.to_csv(output_file_path, sep=';', index=False)
    pass

def main():
    configs_path = sys.argv[1]
    configs = json.load(open(configs_path))

    input_paths = configs['inputFiles']
    output_path = configs['outputFile']
    metrics = configs['desiredMetrics']
    graphId= configs['inputFiles']['columnNames']['graphId']
    graphValue = configs['inputFiles']['columnNames']['graphValue']
    metric_list = [metric_entry['metric'] for metric_entry in metrics]
    
    general_data, graph_data, cliques_data = read_files(input_paths)

    inputs = {
        "graphData": graph_data,
        "cliquesData": cliques_data,
        "generalData": general_data
    }

    metric_generators = get_metrics_dict()
    all_metrics = {}

    for index, row in general_data.iterrows():
        id = row[graphId]
        value = row[graphValue]

        generated_metrics = [id]

        # Here we iterate over the metrics and, for each one, choose the necessary inputs 
        # and call the appropriate generator function, which returns the metric value
        for metric_entry in metrics:
            metric_name = metric_entry['metric']
            required_inputs = []
            required_input_names = metric_entry['requiredInputs']
            for input_name in required_input_names:
                # Some keys are ints and some are strings
                input_for_id = (inputs[input_name].get(id) or 
                                inputs[input_name].get(str(id)))
                required_inputs.append(input_for_id)

            generated_metrics.append(metric_generators[metric_name](*required_inputs))

        generated_metrics.append(value)
        all_metrics[id] = generated_metrics

    output_metrics(all_metrics, output_path, metric_list)

main()