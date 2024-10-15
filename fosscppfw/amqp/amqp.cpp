#include "amqp.h"

namespace AMQP {

std::vector<QueueConfig> generateXRandomQueues(std::string const& exchangeName, int count, MQHandler handler) {
	std::vector<QueueConfig> queues;
	if (count <= 0) {
		return {};
	}
	for (int i = 0; i < count; i++) {
		queues.push_back(
			QueueConfig {
				/* name */ exchangeName + "-queue-" + std::to_string(i + 1),
				/* durable */ true,
				/* autoDelete */ false,
				/* messageTtl */ 0,
				exchangeName,
				/* routingKey */ "",
				handler,
			}
		);
	}
	return queues;
}

} // namespace AMQP