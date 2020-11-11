import sys

import ray
import pytest
from ray.test_utils import (
    generate_system_config_map,
    wait_for_condition,
    wait_for_pid_to_exit,
)


@ray.remote
class Increase:
    def method(self, x):
        return x + 2


@ray.remote
def increase(x):
    return x + 1


@pytest.mark.parametrize(
    "ray_start_regular", [
        generate_system_config_map(
            num_heartbeats_timeout=20, ping_gcs_rpc_server_max_retries=60)
    ],
    indirect=True)
def test_gcs_server_restart(ray_start_regular):
    actor1 = Increase.remote()
    result = ray.get(actor1.method.remote(1))
    assert result == 3

    ray.worker._global_node.kill_gcs_server()
    ray.worker._global_node.start_gcs_server()

    result = ray.get(actor1.method.remote(7))
    assert result == 9

    actor2 = Increase.remote()
    result = ray.get(actor2.method.remote(2))
    assert result == 4

    result = ray.get(increase.remote(1))
    assert result == 2


@pytest.mark.parametrize(
    "ray_start_regular", [
        generate_system_config_map(
            num_heartbeats_timeout=20, ping_gcs_rpc_server_max_retries=60)
    ],
    indirect=True)
def test_gcs_server_restart_during_actor_creation(ray_start_regular):
    ids = []
    # We reduce the number of actors because there are too many actors created
    # and `Too many open files` error will be thrown.
    for i in range(0, 20):
        actor = Increase.remote()
        ids.append(actor.method.remote(1))

    ray.worker._global_node.kill_gcs_server()
    ray.worker._global_node.start_gcs_server()

    ready, unready = ray.wait(ids, num_returns=20, timeout=240)
    print("Ready objects is {}.".format(ready))
    print("Unready objects is {}.".format(unready))
    assert len(unready) == 0


@pytest.mark.parametrize(
    "ray_start_cluster_head", [
        generate_system_config_map(
            num_heartbeats_timeout=20, ping_gcs_rpc_server_max_retries=60)
    ],
    indirect=True)
def test_node_failure_detector_when_gcs_server_restart(ray_start_cluster_head):
    """Checks that the node failure detector is correct when gcs server restart.

    We set the cluster to timeout nodes after 2 seconds of heartbeats. We then
    kill gcs server and remove the worker node and restart gcs server again to
    check that the removed node will die finally.
    """
    cluster = ray_start_cluster_head
    worker = cluster.add_node()
    cluster.wait_for_nodes()

    # Make sure both head and worker node are alive.
    nodes = ray.nodes()
    assert len(nodes) == 2
    assert nodes[0]["alive"] and nodes[1]["alive"]

    to_be_removed_node = None
    for node in nodes:
        if node["RayletSocketName"] == worker.raylet_socket_name:
            to_be_removed_node = node
    assert to_be_removed_node is not None

    head_node = cluster.head_node
    gcs_server_process = head_node.all_processes["gcs_server"][0].process
    gcs_server_pid = gcs_server_process.pid
    # Kill gcs server.
    cluster.head_node.kill_gcs_server()
    # Wait to prevent the gcs server process becoming zombie.
    gcs_server_process.wait()
    wait_for_pid_to_exit(gcs_server_pid, 1000)

    raylet_process = worker.all_processes["raylet"][0].process
    raylet_pid = raylet_process.pid
    # Remove worker node.
    cluster.remove_node(worker, allow_graceful=False)
    # Wait to prevent the raylet process becoming zombie.
    raylet_process.wait()
    wait_for_pid_to_exit(raylet_pid)

    # Restart gcs server process.
    cluster.head_node.start_gcs_server()

    def condition():
        nodes = ray.nodes()
        assert len(nodes) == 2
        for node in nodes:
            if node["NodeID"] == to_be_removed_node["NodeID"]:
                return not node["alive"]
        return False

    # Wait for the removed node dead.
    wait_for_condition(condition, timeout=10)


@pytest.mark.parametrize(
    "ray_start_regular", [
        generate_system_config_map(
            num_heartbeats_timeout=20, ping_gcs_rpc_server_max_retries=60)
    ],
    indirect=True)
def test_del_actor_after_gcs_server_restart(ray_start_regular):
    actor = Increase.options(name="abc").remote()
    result = ray.get(actor.method.remote(1))
    assert result == 3

    ray.worker._global_node.kill_gcs_server()
    ray.worker._global_node.start_gcs_server()

    actor_id = actor._actor_id.hex()
    del actor

    def condition():
        actor_status = ray.actors(actor_id=actor_id)
        if actor_status["State"] == ray.gcs_utils.ActorTableData.DEAD:
            return True
        else:
            return False

    # Wait for the actor dead.
    wait_for_condition(condition, timeout=10)

    # If `PollOwnerForActorOutOfScope` was successfully called,
    # name should be properly deleted.
    with pytest.raises(ValueError):
        ray.get_actor("abc")


@pytest.mark.parametrize(
    "ray_start_cluster_head", [
        generate_system_config_map(
            num_heartbeats_timeout=20, ping_gcs_rpc_server_max_retries=60)
    ],
    indirect=True)
def test_create_pg_after_gcs_server_restart(ray_start_cluster_head):
    cluster = ray_start_cluster_head
    cluster.add_node(num_cpus=4)
    cluster.add_node(num_cpus=4)
    cluster.wait_for_nodes()

    # Create placement group 1 successfully.
    placement_group1 = ray.util.placement_group([{"CPU": 2}, {"CPU": 2}])
    ray.get(placement_group1.ready(), timeout=2)

    # Restart gcs server.
    cluster.head_node.kill_gcs_server()
    cluster.head_node.start_gcs_server()

    # Create placement group 2 successfully.
    placement_group2 = ray.util.placement_group([{"CPU": 2}, {"CPU": 2}])
    ray.get(placement_group2.ready(), timeout=2)

    # Create placement group 3.
    # Status is `PENDING` because the cluster resource is insufficient.
    placement_group3 = ray.util.placement_group([{"CPU": 2}, {"CPU": 2}])
    with pytest.raises(ray.exceptions.GetTimeoutError):
        ray.get(placement_group3.ready(), timeout=2)
    table = ray.util.placement_group_table(placement_group3)
    assert table["state"] == "PENDING"


@pytest.mark.parametrize(
    "ray_start_cluster_head", [
        generate_system_config_map(
            num_heartbeats_timeout=20, ping_gcs_rpc_server_max_retries=60)
    ],
    indirect=True)
def test_create_actor_with_pg_after_gcs_server_restart(ray_start_cluster_head):
    cluster = ray_start_cluster_head
    cluster.add_node(num_cpus=4)
    cluster.wait_for_nodes()

    # Create a placement group.
    placement_group = ray.util.placement_group([{"CPU": 2}, {"CPU": 2}])

    # Create an actor that occupies resources after gcs server restart.
    cluster.head_node.kill_gcs_server()
    cluster.head_node.start_gcs_server()
    actor_2 = Increase.options(
        placement_group=placement_group,
        placement_group_bundle_index=1).remote()
    assert ray.get(actor_2.method.remote(1)) == 3


@pytest.mark.parametrize(
    "ray_start_cluster_head", [
        generate_system_config_map(
            num_heartbeats_timeout=20, ping_gcs_rpc_server_max_retries=60)
    ],
    indirect=True)
def test_create_pg_during_gcs_server_restart(ray_start_cluster_head):
    cluster = ray_start_cluster_head
    cluster.add_node(num_cpus=4)
    cluster.wait_for_nodes()

    # Create a placement group during gcs server restart.
    cluster.head_node.kill_gcs_server()
    placement_group = ray.util.placement_group([{"CPU": 2}, {"CPU": 2}])
    cluster.head_node.start_gcs_server()
    ray.get(placement_group.ready(), timeout=2)

    # Create an actor that occupies resources.
    actor = Increase.options(
        placement_group=placement_group,
        placement_group_bundle_index=0).remote()
    assert ray.get(actor.method.remote(1)) == 3


if __name__ == "__main__":
    import pytest
    sys.exit(pytest.main(["-v", __file__]))
