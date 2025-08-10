export async function onRequest(context) {
    const method = context.request.method;

    const allowedOrigins = [
        "http://localhost",
        "https://hitcon.org"
    ];

    if (context.request.headers.has("Origin") && !allowedOrigins.includes(context.request.headers.get("Origin"))) {
        return new Response(
            JSON.stringify({"detail": "Origin not allowed"}),
            {
                status: 403,
                headers: {
                    "Access-Control-Allow-Origin": "https://hitcon.org"
                }
            }
        );
    }

    if (method !== "GET" && method !== "POST") {
        return new Response(
            JSON.stringify({"detail": "Method Not Allowed"}),
            {
                status: 405,
                headers: {
                    "Allow": "GET, POST",
                    "Content-Type": "application/json"
                }
            }
        );
    }

    if (!context.request.headers.has("Authorization")) {
        return new Response(
            JSON.stringify({"detail": "Unauthorized"}),
            {
                status: 401,
                headers: {
                    "WWW-Authenticate": "Bearer",
                    "Content-Type": "application/json"
                }
            }
        );
    }

    if (
        method === "POST" &&
        (!context.request.headers.has("Content-Type") || context.request.headers.get("Content-Type") !== "application/json")
    ) {
        return new Response(
            JSON.stringify({"detail": "Bad Request"}),
            {
                status: 400,
                headers: {
                    "Content-Type": "application/json"
                }
            }
        );
    }

    const backendUrl = `${context.env.BACKEND}/hitcon/link`;
    const headers = new Headers(context.request.headers);
    headers.set("Authorization", context.request.headers.get("Authorization"));
    headers.set("Content-Type", "application/json");

    const requestOptions = {
        method: method,
        headers: headers
    };

    if (method === "POST") {
        requestOptions.body = JSON.stringify(await context.request.json());
    }

    return await fetch(backendUrl, requestOptions);
}
