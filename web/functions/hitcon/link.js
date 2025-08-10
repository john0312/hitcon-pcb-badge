const allowedOrigins = [
    "http://localhost",
    "https://hitcon.org"
];

function isAllowedOrigin(origin) {
    return allowedOrigins.includes(origin);
}

function getAccessControlAllowOriginHeader(origin) {
    origin = origin?.toLowerCase();
    if (isAllowedOrigin(origin)) {
        return {"Access-Control-Allow-Origin": origin, "Vary": "Origin"};
    }
    return {};
}

function getCORSHeaders(origin) {
    origin = origin?.toLowerCase();

    const headers = new Headers();
    headers.set("Access-Control-Allow-Origin", origin);
    headers.set("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    headers.set("Access-Control-Allow-Headers", "Authorization, Content-Type");
    headers.set("Access-Control-Allow-Credentials", "true");
    headers.set("Access-Control-Max-Age", "3600"); // 1 hour
    return headers;
}

export async function onRequest(context) {
    const method = context.request.method;
    const origin = context.request.headers.get("Origin");

    // Allow direct API calls
    if (origin) {
        if (!isAllowedOrigin(origin)) {
            return new Response(null, {
                status: 403
            });
        }

        if (method === "OPTIONS") {
            return new Response(null, {
                status: 204,
                headers: getCORSHeaders(origin)
            });
        }
    }

    if (method !== "GET" && method !== "POST") {
        return new Response(
            JSON.stringify({"detail": "Method Not Allowed"}),
            {
                status: 405,
                headers: {
                    "Allow": "GET, POST",
                    "Content-Type": "application/json",
                    ...getAccessControlAllowOriginHeader(origin)
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
                    "Content-Type": "application/json",
                    ...getAccessControlAllowOriginHeader(origin)
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
                    "Content-Type": "application/json",
                    ...getAccessControlAllowOriginHeader(origin)
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

    const res = (await fetch(backendUrl, requestOptions)).clone();
    if (origin && isAllowedOrigin(origin)) {
        res.headers.set("Access-Control-Allow-Origin", origin.toLowerCase());
    }

    return res;
}
