import bcrypt from "bcryptjs"
import jwt from "jsonwebtoken"
import { ObjectId } from "mongodb"
import { env } from "@/lib/env"
import { getDb } from "@/lib/db"

export type PublicUser = {
  _id: string
  email: string
  name?: string
}

export async function createUser(email: string, password: string, name?: string) {
  const { db } = await getDb()
  const existing = await db.collection("users").findOne({ email })
  if (existing) throw new Error("Email already in use")
  const hash = await bcrypt.hash(password, 10)
  const res = await db.collection("users").insertOne({ email, password: hash, name, createdAt: new Date() })
  return { _id: res.insertedId.toString(), email, name } as PublicUser
}

export async function verifyUser(email: string, password: string) {
  const { db } = await getDb()
  const user = await db.collection("users").findOne<{ _id: ObjectId; email: string; password: string; name?: string }>({ email })
  if (!user) throw new Error("Invalid credentials")
  const ok = await bcrypt.compare(password, user.password)
  if (!ok) throw new Error("Invalid credentials")
  return { _id: user._id.toString(), email: user.email, name: user.name } as PublicUser
}

export function signToken(user: PublicUser) {
  return jwt.sign({ sub: user._id, email: user.email }, env.jwtSecret, { expiresIn: "7d" })
}

export function verifyToken(token: string) {
  return jwt.verify(token, env.jwtSecret) as { sub: string; email: string; iat: number; exp: number }
}



