import { NextResponse } from "next/server"
import type { NextRequest } from "next/server"

const PROTECTED_PATHS = [
  /^\/orders($|\/)/,
  /^\/orderbook($|\/)/,
  /^\/profile($|\/)/,
]

export function middleware(req: NextRequest) {
  const { pathname } = req.nextUrl
  const isProtected = PROTECTED_PATHS.some((re) => re.test(pathname))
  if (!isProtected) return NextResponse.next()

  const token = req.cookies.get("token")?.value
  if (!token) {
    const url = new URL("/login", req.url)
    url.searchParams.set("next", pathname)
    return NextResponse.redirect(url)
  }
  return NextResponse.next()
}

export const config = {
  matcher: ["/orders/:path*", "/orderbook/:path*", "/profile/:path*"],
}


