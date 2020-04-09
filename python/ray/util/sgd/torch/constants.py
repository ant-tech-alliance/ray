from ray.ray_constants import env_integer

USE_FP16 = "__use_fp16__"
NUM_STEPS = "__num_steps__"
SCHEDULER_STEP = "scheduler_step"
SCHEDULER_STEP_BATCH = "batch"
SCHEDULER_STEP_EPOCH = "epoch"
NCCL_TIMEOUT_S = env_integer("NCCL_TIMEOUT_S", 10)

VALID_SCHEDULER_STEP = {SCHEDULER_STEP_BATCH, SCHEDULER_STEP_EPOCH}
