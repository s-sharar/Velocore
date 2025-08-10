import { NextResponse } from "next/server"
import { cookies } from "next/headers"
import { verifyToken } from "@/lib/auth"
import { getDb } from "@/lib/db"
import { ObjectId } from "mongodb"

export async function GET() {
  try {
    const token = cookies().get("token")?.value
    if (!token) return NextResponse.json({ user: null })
    const payload = verifyToken(token)
    const { db } = await getDb()
    const user = await db
      .collection("users")
      .findOne<{ _id: ObjectId; email: string; name?: string }>({ _id: new ObjectId(payload.sub) }, { projection: { password: 0 } })
    if (!user) return NextResponse.json({ user: null })
    return NextResponse.json({ user: { _id: user._id.toString(), email: user.email, name: user.name } })
  } catch {
    return NextResponse.json({ user: null })
  }
}



