/*
*
*	MIT License
*
*	Copyright(c) 2018 ZiiCreater LLC
*
*	Permission is hereby granted, free of charge, to any person obtaining a copy
*	of this software and associated documentation files(the "Software"), to deal
*	in the Software without restriction, including without limitation the rights
*	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*	copies of the Software, and to permit persons to whom the Software is
*	furnished to do so, subject to the following conditions :
*
*	The above copyright notice and this permission notice shall be included in all
*	copies or substantial portions of the Software.
*
*	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
*	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
*	SOFTWARE.
*
*/

#include "SocketClusterResponse.h"
#include "SocketClusterModule.h"

SocketClusterResponse::SocketClusterResponse(double cid, FResponseCallback Callback, const FLatentActionInfo& LatentInfo) : CID(cid), AckTimeout(USocketClusterClient::AckTimeout), ExecutionFunction(LatentInfo.ExecutionFunction), OutputLink(LatentInfo.Linkage), CallbackTarget(LatentInfo.CallbackTarget), Callback(Callback)/* , Result(ResultParam) */
{

}

void SocketClusterResponse::UpdateOperation(FLatentResponse& Response)
{

	bool completed = false;

	// Are we requesting a acknowledgement.
	if (CID != NULL)
	{
		// Count the current ElapsedTime
		AckTimeout -= Response.ElapsedTime();

		// Check if the reponse has received
		bool exists = USocketClusterClient::Responses.Contains(CID);
		if (exists)
		{
			// Response has been received get the result and delete it from the buffer.
			FString Result;
			USocketClusterClient::Responses.RemoveAndCopyValue(CID, Result);

			// Excute the Callback event.
			Callback.ExecuteIfBound(Result);
			completed = true;
		}

		// Response took to long cancel Latent by setting error and completing.
		if (AckTimeout <= 0.0f)
		{
			// Set the error.
			FString Result = "AckTimeOut Error";

			// Excute the Callback event.
			Callback.ExecuteIfBound(Result);
			completed = true;
		}

		// Trigger LatentAction
		Response.FinishAndTriggerIf(completed, ExecutionFunction, OutputLink, CallbackTarget);
	}
	else
	{
		// we are not requesting a acknowledgement continue as normal.
		Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
	}
	
	
}
