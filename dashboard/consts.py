DASHBOARD_LOG_FILENAME = "dashboard.log"
DASHBOARD_AGENT_PORT_PREFIX = "DASHBOARD_AGENT_PORT_PREFIX:"
DASHBOARD_AGENT_LOG_FILENAME = "dashboard_agent.log"
DASHBOARD_AGENT_CHECK_PARENT_INTERVAL_SECONDS = 2
MAX_COUNT_OF_GCS_RPC_ERROR = 10
UPDATE_NODES_INTERVAL_SECONDS = 5
CONNECT_GCS_INTERVAL_SECONDS = 2
PURGE_DATA_INTERVAL_SECONDS = 60 * 10
REDIS_KEY_DASHBOARD = "dashboard"
REDIS_KEY_GCS_SERVER_ADDRESS = "GcsServerAddress"
REPORT_METRICS_TIMEOUT_SECONDS = 2
REPORT_METRICS_INTERVAL_SECONDS = 10
# Named signals
SIGNAL_NODE_INFO_FETCHED = "node_info_fetched"
# Default param for RotatingFileHandler
LOGGING_ROTATE_BYTES = 100 * 1000 * 1000  # maxBytes
LOGGING_ROTATE_BACKUP_COUNT = 5  # backupCount
