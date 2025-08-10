import { NextResponse } from "next/server"
import { z } from "zod"
import { createUser, signToken } from "@/lib/auth"

const schema = z.object({
  email: z.string().email(),
  password: z.string().min(6),
  name: z.string().optional(),
})

export async function POST(req: Request) {
  try {
    const body = await req.json()
    const { email, password, name } = schema.parse(body)
    const user = await createUser(email, password, name)
    const token = signToken(user)
    const res = NextResponse.json({ user })
    res.cookies.set("token", token, { httpOnly: true, sameSite: "lax", path: "/" })
    return res
  } catch (e: any) {
    return NextResponse.json({ error: e.message || "Registration failed" }, { status: 400 })
  }
}



