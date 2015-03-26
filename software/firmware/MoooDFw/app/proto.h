
#ifndef PROTO_H_
#define PROTO_H_

void loop_tick(systime_t timeout);

typedef enum command
{
	/* OUT commands */
	OFF_CMD = 0,
	ON_CMD  = 1,
	RGB_CMD = 2

} command_t;

typedef struct header
{
	uint8_t		command;
	uint8_t		len;
} header_t;

typedef struct data_pkt
{
	uint32_t	red_dc;
	uint32_t	green_dc;
	uint32_t	blue_dc;
} data_pkt_t;

typedef struct pkt
{
	union
	{
		struct
		{
			header_t	header;
			union
			{
				data_pkt_t	rgb_data;
			};
		};
		uint8_t dummy[64]; // OUT_REPORT_SIZE
	};
} pkt_t;


#endif /* PROTO_H_ */
