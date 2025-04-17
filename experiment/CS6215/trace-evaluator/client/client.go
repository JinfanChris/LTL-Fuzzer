package client

import (
	"context"
	"time"

	pb "tracor/gen/ltlfuzz"

	"github.com/sirupsen/logrus"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
)

type LTLFuzzClient struct {
	conn   *grpc.ClientConn
	client pb.FuzzServiceClient
}

// NewLTLFuzzClient creates and connects to the gRPC server
func NewLTLFuzzClient(addr string) (c *LTLFuzzClient, err error) {
	conn, err := grpc.NewClient(addr, grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		logrus.Errorf("failed to create client: %v", err)
		return
	}

	client := pb.NewFuzzServiceClient(conn)

	return &LTLFuzzClient{conn: conn, client: client}, nil
}

// Close closes the gRPC connection
func (c *LTLFuzzClient) Close() error {
	return c.conn.Close()
}

// PrepareLTL sends LTL properties to the server
func (c *LTLFuzzClient) PrepareLTL(properties []string) (string, error) {
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()

	resp, err := c.client.PrepareLTLProperties(ctx, &pb.LTLList{
		Properties: properties,
	})
	if err != nil {
		return "", err
	}
	return resp.Message, nil
}

// SubmitTrace sends a trace and returns whether it satisfies the properties
func (c *LTLFuzzClient) SubmitTrace(trace string) (bool, []string, error) {
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()

	resp, err := c.client.SubmitTrace(ctx, &pb.TraceInput{
		Trace: trace,
	})
	if err != nil {
		return false, nil, err
	}
	return resp.Satisfied, resp.Violations, nil
}
